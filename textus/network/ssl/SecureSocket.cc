/* SecureSocket.h -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * Represents a network socket secured via TLS.
 *
 */

/*
 *   This program is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 3 of the
 *   License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see
 *   <http://www.gnu.org/licenses/>.
 *
 */

#include "textus/network/ssl/SecureSocket.h"
#include "textus/network/ssl/SSLVerifier.h"
#include "textus/random/PRNG.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace network { namespace ssl {
#include "openssl/rand.h"

DEFINE_STRING_ARG(SSL_default_ca_path, "$(APP_PATH)/etc/ssl/CA", "ssl_default_ca_path", "The default path for the list of trusted certificate authorities.");
DEFINE_STRING_ARG(SSL_default_certificate_path, "$(APP_PATH)/etc/ssl/certificate.pem", "ssl_default_certificate_path", "The default path for the certificate for this app.");
DEFINE_STRING_ARG(SSL_default_dh_params_path, "$(APP_PATH)/etc/ssl/dh_param.pem", "ssl_default_certificate_path", "The default path for the certificate for this app.");
DEFINE_STRING_ARG(SSL_default_ciphers, "HIGH", "ssl_default_ciphers", "The default ciphers for this app.");
DEFINE_STRING_ARG(SSL_default_key_path, "$(APP_PATH)/etc/ssl/key.pem", "ssl_default_key_path", "The default path for the private key for this app.");
DEFINE_STRING_ARG(SSL_default_key_pass, "test", "ssl_default_key_pass", "The default password for the apps private key");
DEFINE_INT_ARG(SSL_random_seed_size, 128, "ssl_random_seed_size", "The size in bytes of the random buffer to seed the SSL random number generator with.");

int SecureSocket::verifier_index = -1;
Base SecureSocket::verifier_index_lock;

void SecureSocket::releaseCTX(SSL_CTX *c) {
  if (c) {
    SSL_CTX_free(c);
  }
}

void SecureSocket::releaseSSL(SSL *s) {
  if (s) {
    SSL_shutdown(s);
    SSL_free(s);
  }
}

void SecureSocket::error(textus::event::Event *se) {
  ERR_print_errors_fp(stderr);
  close();
  watch_read = false;
  watch_write = false;
  Socket::error(se);
}


int SecureSocket::verify_callback(int preverify_ok, X509_STORE_CTX *cts) {
  Synchronized(this);
  if (verifier == NULL) {
    return preverify_ok;
  } else {
    return verifier->verify(this, preverify_ok, cts);
  }
}

int SecureSocket::ssl_verify_callback(int preverify_ok, X509_STORE_CTX *cts) {
  SecureSocket *ss;
  {
    Synchronized(&verifier_index_lock);
    if (verifier_index == -1) {
      return preverify_ok;
    }

    SSL *ssl = static_cast<SSL *>(X509_STORE_CTX_get_ex_data(cts, SSL_get_ex_data_X509_STORE_CTX_idx()));
    ss = static_cast<SecureSocket *>(SSL_get_ex_data(ssl, verifier_index));
  }

  if (ss) {
    return ss->verify_callback(preverify_ok, cts);
  }
  return preverify_ok;
}

int SecureSocket::verifier_new_func(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp) {
  CRYPTO_set_ex_data(ad, idx, NULL);
  return 0;
}

int SecureSocket::verifier_dup_func(CRYPTO_EX_DATA *to, CRYPTO_EX_DATA *from, void *from_d, int idx, long argl, void *argp) {
  LOG(ERROR) << "SecureSocket::verifier_dup_func called.\n";
  die();
  return 1;
}

void SecureSocket::verifier_free_func(void *parent, void *ptr, CRYPTO_EX_DATA *ad, int idx, long argl, void *argp) {
  SecureSocket *ss = static_cast<SecureSocket *>(CRYPTO_get_ex_data(ad, idx));
  CRYPTO_set_ex_data(ad, idx, NULL);
  if (ss) {
    ss->weakDeref();
  }
}

int SecureSocket::passwdCB(char *buf, int size, int rwflag, void *userdata)
{
  SecureSocket *ss = reinterpret_cast<SecureSocket *>(userdata);
  return ss->passwordCB(buf, size, rwflag);
}

int SecureSocket::passwordCB(char *buf, int size, int rwflag) {
  strncpy(buf, key_pass.c_str(), size);
  buf[size-1] = 0;
  return strlen(buf);

}


SecureSocket::~SecureSocket()
{
  close();
  verifier = NULL;
  ssl = NULL;
  ctx = NULL;
  // Now we have to make sure that the certificate isn't refreed.
  certificate.clearNoDeref();
}

DEFINE_INIT_FUNCTION(SecureSocketInit, NETWORK_INIT_PRIORITY + 1)
{
  unsigned char seed[SSL_random_seed_size]; //some entropy to get the random number generator started.
  SSL_load_error_strings();
  SSL_library_init();
  OpenSSL_add_all_ciphers();
  OpenSSL_add_all_digests();
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  textus::system::SysInfo::systemInformation()->random(seed, sizeof(seed));
  textus::random::PRNG::globalPRNG()->seed();
  for (unsigned i = 0; i < sizeof(seed); i++) {
    seed[i] ^= textus::random::PRNG::globalPRNG()->rand8();
  }
  RAND_seed(seed, sizeof(seed));
  return 0;
}


}}} //namespace
