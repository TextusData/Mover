/* SecureSocket.h -*- c++ -*-
 * Copyright (c) 2010-2013 Ross Biro
 *
 * Represents a network socket secured via TLS.
 *
 */

#ifndef TEXTUS_NETWORK_SSL_SECURE_SOCKET_H_
#define TEXTUS_NETWORK_SSL_SECURE_SOCKET_H_

#include "textus/network/Socket.h"
#include "textus/network/ssl/Certificate.h"
#include "textus/network/ssl/SSLVerifier.h"
#include "textus/system/Environment.h"

#include <stdio.h>

namespace textus { namespace network { namespace ssl {

extern "C" {
#include "openssl/ssl.h"
#include "openssl/dh.h"
#include "openssl/pem.h"

  extern void ERR_print_errors_fp(FILE *);
}

#define DEFAULT_SSL_VERIFY_FLAGS (SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE)
#define DEFAULT_SSL_OPTION_FLAGS (SSL_OP_SINGLE_DH_USE)

DECLARE_STRING_ARG(SSL_default_ca_path);
DECLARE_STRING_ARG(SSL_default_certificate_path);
DECLARE_STRING_ARG(SSL_default_dh_params_path);
DECLARE_STRING_ARG(SSL_default_ciphers);
DECLARE_STRING_ARG(SSL_default_key_path);
DECLARE_STRING_ARG(SSL_default_key_pass);

class SecureSocket: public Socket {
private:
  static void releaseCTX(SSL_CTX *);
  static void releaseSSL(SSL *);
  AutoFoo<SSL_CTX, releaseCTX> ctx;
  AutoFoo<SSL, releaseSSL> ssl;
  int verify_flags;
  static int verifier_index;
  static Base verifier_index_lock;
  AutoDeref<class SSLVerifier> verifier;
  AutoDeref<Certificate> peerCertificate;
  MVAR(public, textus::system::Environment, environment)
  AutoDeref<Certificate> certificate;
  string key_pass;

  static int ssl_verify_callback(int preverify_ok, X509_STORE_CTX *cts);
  static int verifier_new_func(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp);
  static int verifier_dup_func(CRYPTO_EX_DATA *to, CRYPTO_EX_DATA *from, void *from_d, int idx, long argl, void *argp);
  static void verifier_free_func(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp);
  AutoFoo<DH, DH_free> dhp;

protected:
  bool connecting;
  bool accepting;

  virtual int weakDelete() 
  {
    peerCertificate = NULL;
    verifier = NULL;
    {
      Synchronized(&verifier_index_lock);
      if (ssl != NULL && verifier_index != -1 ) {
	SecureSocket *ss = static_cast<SecureSocket *>(SSL_get_ex_data(ssl, verifier_index));
	SSL_set_ex_data(ssl, verifier_index, NULL);
	if (ss != NULL) {
	  ss->weakDeref();
	}
      }
    }
    return Socket::weakDelete();
  }

  void processReadWriteError(int ret) {
    Synchronized(this);
    ret = SSL_get_error(ssl, ret);
    switch(ret) {
    case SSL_ERROR_WANT_READ:
      watch_read = true;
      watch_write = false;
      break;

    case SSL_ERROR_WANT_WRITE:
      watch_read = false;
      watch_write = true;
      break;

    case SSL_ERROR_SYSCALL:
      fprintf(stderr, "fd=%d errno=%d\n", nativeHandle(), errno);
      LOG(ERROR) << "SSL failed for fd(" << nativeHandle() <<") with errno="
		 << errno << "\n" << std::endl;
      error(NULL);
      close();
      break;

    default:
      fprintf(stderr, "fd = %d: ", nativeHandle());
      ERR_print_errors_fp(stderr);
      error(NULL);
      close();
    }
  }

  int processConnect() {
    EXEC_BARRIER();
    Synchronized(this);
    int ret = SSL_connect(ssl);
    if (ret < 0) {
      processReadWriteError(ret);
      return ret;
    }
    connecting = false;
    connected = true;
    signal();
    return 0;
  }

  int processAccept() {
    Synchronized(this);
    EXEC_BARRIER();
    int ret = SSL_accept(ssl);
    if (ret < 0) {
      processReadWriteError(ret);
      return ret;
    }
    accepting = false;
    connected = true;
    signal();
    return 0;
  }

  static int passwdCB(char *buf, int size, int rwflag, void *password);
  virtual int passwordCB(char *buf, int size, int rwflag);

  void setCertificate(Certificate *cert) {
    certificate = cert;
  }

  int initSSLObject(bool load_certificate=true) {
    Synchronized(this);

    SSL_CTX_set_options(ctx, DEFAULT_SSL_OPTION_FLAGS);
    SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    SSL_CTX_set_verify(ctx, DEFAULT_SSL_VERIFY_FLAGS, NULL);

    if (SSL_CTX_set_cipher_list(ctx, SSL_default_ciphers.c_str()) == 0) {
      LOG(ERROR) << "No Ciphers Available." << "\n";
      ERR_print_errors_fp(stderr);
      die();
    }
    loadDHParams(environment()->expandStrings(SSL_default_dh_params_path));

    SSL_CTX_set_default_passwd_cb(ctx, passwdCB);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, this);

    if (load_certificate) {
      string key_path = environment()->expandStrings(SSL_default_key_path);
      if (SSL_CTX_use_PrivateKey_file(ctx, key_path.c_str(), SSL_FILETYPE_PEM) != 1) {
	ERR_print_errors_fp(stderr);
	die();
      }

      string cert_path = environment()->expandStrings(SSL_default_certificate_path);
      if (SSL_CTX_use_certificate_chain_file(ctx, cert_path.c_str()) != 1) {
	ERR_print_errors_fp(stderr);
	die();
      }
      /*Make sure the key and certificate file match*/
      if (SSL_CTX_check_private_key(ctx) == 0) {
	ERR_print_errors_fp(stderr);
	die();
      }
    }

    string ca_path = environment()->expandStrings(SSL_default_ca_path);
    if (SSL_CTX_load_verify_locations(ctx, NULL, ca_path.c_str()) == 0) {
      ERR_print_errors_fp(stderr);
      die();
    }

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
      return -1;
    }

    if (nativeHandle() >= 0) {
      return SSL_set_fd(ssl, nativeHandle());
    }

    return 0;
  }

  virtual int read(int fd, void *buff, size_t buff_size) {
    Synchronized(this);
    int ret;
    if (hadError()) {
      return -1;
    }
    if (connecting) {
      processConnect();
      errno = EAGAIN;
      return -1;
    } else if (accepting) {
      processAccept();
      errno = EAGAIN;
      return -1;
    } else {
      EXEC_BARRIER();
      ret=SSL_read(ssl, buff, buff_size);
      if (ret < 0) {
	processReadWriteError(ret);
	errno = EAGAIN;
	return -1;
      }
    }
    return ret;
  }

  virtual int write(int fd, const void *buff, size_t buff_size) {
    Synchronized(this);
    int ret;
    if (connecting) {
      processConnect();
      errno = EAGAIN;
      return -1;
    } else if (accepting) {
      processAccept();
      errno = EAGAIN;
      return -1;
    } else {
      EXEC_BARRIER();
      ret=SSL_write(ssl, buff, buff_size);
    }
    if (ret < 0) {
      processReadWriteError(ret);
      errno = EAGAIN;
      return -1;
    }
    return ret;
  }

  virtual int verify_callback(int preverify_ok, X509_STORE_CTX *cts);

public:
  SecureSocket(): verify_flags(DEFAULT_SSL_VERIFY_FLAGS), key_pass(SSL_default_key_pass), connecting(false), accepting(false) {
    setenvironment(&textus::system::Environment::mutableSystemEnvironment());
  }
  virtual ~SecureSocket();

  virtual void openNoLock(int f) {
    if (!SSL_set_fd(ssl, f)) {
      ERR_print_errors_fp(stderr);
      assert(0);
    }
    Socket::openNoLock(f);
  }

  virtual void open(int f) {
    Synchronized(this);
    SSL_set_fd(ssl, f);
    Socket::open(f);
  }

  int init(bool load_certificate=true) {
    Synchronized(this);
    ctx = SSL_CTX_new(SSLv23_method());
    if (ctx == NULL) {
      ERR_print_errors_fp(stderr);
      return -1;
    }
    return initSSLObject(load_certificate);
  }

  int setVerifier(class SSLVerifier *cv) {
    {
      Synchronized(&verifier_index_lock);
      if (verifier_index == -1) {
	// need a verifier index;
	verifier_index = SSL_get_ex_new_index(-1, this, verifier_new_func, verifier_dup_func, verifier_free_func);
      } 
    }
    int ret = 0;
    Synchronized(this);
    SSL_set_verify(ssl, verify_flags, ssl_verify_callback);
    SecureSocket *ss = static_cast<SecureSocket *>(SSL_get_ex_data(ssl, verifier_index));
    if (ss) {
      ss->weakDeref();
    }
    ret = SSL_set_ex_data(ssl, verifier_index, this);
    if (ret) {
      this->weakRef();
    }
    verifier = cv;

    return ret;
  }

  void setVerifyFlags(int flags) {
    {
      Synchronized(this);
      verify_flags = flags;
    }
    setVerifier(verifier); // make sure the flags get propogated to the ssl layer.
  }

  virtual void close() {
    Synchronized(this);
    {
      Synchronized(&verifier_index_lock);
      if (ssl != NULL && verifier_index != -1 ) {
	SecureSocket *ss = static_cast<SecureSocket *>(SSL_get_ex_data(ssl, verifier_index));
	SSL_set_ex_data(ssl, verifier_index, NULL);
	if (ss) {
	  ss->weakDeref();
	}
      }
    }
    peerCertificate = NULL;
    setenvironment(NULL);
    Socket::close();
  }

  virtual int connect(NetworkAddress *addr) {
    int ret = 0;
    Synchronized(this);
    HRC(Socket::connect(addr));
    connecting = true;
  error_out:
    return ret;
  }

  virtual int connect(URL *u) {
    return Socket::connect(u);
  }

  //XXXXXX Important.  It's important that each certificate only
  // be used by 1 connection.  We really need an easy way to clone
  // X509 certificates.
  int useCertificate(Certificate *c) {
    AUTODEREF(Certificate *, c1);
    if (c == NULL) {
      c1 = Certificate::random();
      c = c1;
    }
    Synchronized(this);
    certificate = c;
    if (SSL_use_certificate(ssl, c->x509()) != 1) {
      return -1;
    }
    return SSL_use_PrivateKey(ssl, c->privateKey());
  }

  int useCertificateFile(string filename) {
    return SSL_use_certificate_file(ssl, filename.c_str(), SSL_FILETYPE_PEM);
  }

  Certificate *getPeerCertificate() {
    if (peerCertificate) 
      return peerCertificate;

    X509 *c = SSL_get_peer_certificate(ssl);
    peerCertificate = new Certificate(c);
    return peerCertificate;
  }

  int setCiphers(const char *str) {
    Synchronized(this);
    if (SSL_set_cipher_list(ssl, str) == 0) {
      ERR_print_errors_fp(stderr);
      return 0;
    }
    return 1;
  }

  int setDHParams(DH *dh) {
    Synchronized(this);
    if (ssl) {
      return SSL_set_tmp_dh(ssl, dh);
    } else {
      return SSL_CTX_set_tmp_dh(ctx, dh);
    }
  }

  int loadDHParams(string filename) {
    // XXXXX Fixme, use textusfile et al.
    EXEC_BARRIER();
    FILE *pf = fopen (filename.c_str(), "r");
    if (pf) {
      dhp = PEM_read_DHparams(pf, NULL, NULL, NULL);
      fclose(pf);
      if (dhp) {
	setDHParams(dhp);
	return 0;
      }
    }
    return 1;
  }

  int initTLS(bool load_certificate=true) {
    Synchronized(this);
    ctx = SSL_CTX_new(TLSv1_method());
    if (ctx == NULL) {
      return -1;
    }
    return initSSLObject(load_certificate);
  }

  virtual void error(textus::event::Event *se);

  virtual void canWrite(textus::event::Event *se) { 
    Synchronized(this); 
    watch_write = false;

    if (connected) {
      dataAvailable();
      return;
    }
      
    if (accepting) {
      if (processAccept() == 0) {
	accepting = false;
	connected = true;
	watch_read = true;
	signal();
      }
      return;
    } else if (!connecting) {
      AUTODEREF(NetworkAddress *, na);
      na = getPeerAddress();
      if (!na) {
	error(se);
	return;
      }
      connecting = true;
    }

    if (processConnect() == 0) {
      connected = true;
      connecting = false;
      watch_read = true;
      signal();
    }
  }

  void acceptSecureConnection() {
    accepting = true;
    if (processAccept() == 0) {
      accepting = false;
      connected = true;
      watch_read = true;
      signal();
    }
  }
};

class SecureSocketFactory: public SocketFactory {
private:
  int verify_flags;

public:
  SecureSocketFactory(): verify_flags(DEFAULT_SSL_VERIFY_FLAGS) {}
  virtual ~SecureSocketFactory() {}

  virtual SecureSocket *createSecureSocket() = 0;

  virtual Socket *createSocket() {
    assert(0);
  }

  virtual Socket *create(Socket *parent, int fd) {
    Synchronized(this);
    SecureSocket *s = createSecureSocket();
    if (verify_flags != DEFAULT_SSL_VERIFY_FLAGS) {
      s->setVerifyFlags(verify_flags);
    }
    s->setParentSocket(parent);
    s->open(fd);
    s->acceptSecureConnection();
    return s;
  }

  int verifyFlags() {
    Synchronized(this);
    return verify_flags;
  }

  void setVerifyFlags(int vf) {
    Synchronized(this);
    verify_flags = vf;
  }

};


}}} // namespace


#endif // TEXTUS_NETWORK_SECURE_SOCKET_H_
