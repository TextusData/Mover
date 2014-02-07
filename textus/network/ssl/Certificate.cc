/* Certificate.cc -*- c++ -*-
 * Copyright (c) 2010-2013 Ross Biro
 *
 * Represents a certificate, really a certificate chain.
 *
 */

#include "textus/network/ssl/Certificate.h"
#include "textus/file/TextusFile.h"

#include <fcntl.h>

#include <string>

#define MAX_CERT_SIZE 16384


#define add_extension(nid, value)			    \
  do {							    \
      X509V3_set_ctx(&ctx, x509, x509, NULL, NULL, 0);	    \
      HRZ(X509V3_EXT_conf_nid(NULL, &ctx, nid, const_cast<char *>(value))); \
      X509_add_ext(x509, ex, -1);			    \
      X509_EXTENSION_free(ex);				    \
      ex = NULL;					    \
  } while(0)

namespace textus { namespace network { namespace ssl {

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

using namespace std;

void Certificate::X509_release(X509 *x) {
  X509_free(x);
}

void Certificate::RSA_release(RSA *x) {
  RSA_free(x);
}

/* use TextusFile here instead of FILE * 
 * so that we can theoretically
 * get the certificate from other systems,
 * that don't support a file
 * system.
 */
Certificate *Certificate::fromFile(string file) {
  AUTODEREF(textus::file::TextusFile *, tf);
  int ret = 0;
  string s;
  tf = textus::file::TextusFile::openFile(file, O_RDONLY);
  HRNULL(tf);
  s = tf->read(MAX_CERT_SIZE);
  tf = NULL;
 error_out:
  if (!ret) {
    return Certificate::fromString(s);
  } else {
    return NULL;
  }
}

Certificate *Certificate::random(int bits, int expires) {
  EVP_PKEY *pk = EVP_PKEY_new();
  X509 *x509 = X509_new();
  X509_EXTENSION *ex = NULL;
  X509V3_CTX ctx;
  RSA *rsa=NULL;
  int serial=0;
  int ret=0;
  X509_NAME *name=NULL;
  Certificate *c = NULL;
  
  HRNULL(pk);
  HRNULL(x509);

  rsa=RSA_generate_key(bits, RSA_F4, NULL, NULL);
  HRNULL(rsa);
  HRZ(EVP_PKEY_assign_RSA(pk, rsa));
  HRZ(X509_set_version(x509, 2));
  HRZ(ASN1_INTEGER_set(X509_get_serialNumber(x509), serial));
  HRZ(X509_gmtime_adj(X509_get_notBefore(x509), 0));
  HRZ(X509_gmtime_adj(X509_get_notAfter(x509), expires));
  HRZ(X509_set_pubkey(x509, pk));
  name=X509_get_subject_name(x509);
  HRNULL(name);
  HRZ(X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, reinterpret_cast<const unsigned char *>("US"), -1, -1, 0));
  HRZ(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char *>("Textus Random Cert"),
				 -1, -1, 0));
  HRZ(X509_set_issuer_name(x509, name));

  // set up the extensions.
  X509V3_set_ctx_nodb(&ctx);
  add_extension(NID_basic_constraints, "critical,CA:TRUE");
  add_extension(NID_key_usage, "critical,keyCertSign,cRLSign");
  add_extension(NID_subject_key_identifier, "hash");
  add_extension(NID_netscape_cert_type, "sslCA");

  // Do the signing.
  HRZ(X509_sign(x509, pk, EVP_sha1()));

  // build the Certificate class.
  c = new Certificate(x509);
  c->setPrivateKey(pk);
  c->setRSA(rsa);
  
  goto done;

 error_out:
  if (x509) {
    X509_free(x509);
  }

  if (pk) {
    EVP_PKEY_free(pk);
  }
  
  if (c) {
    delete c;
    c = NULL;
  }

 done:

  if (ex) {
    X509_EXTENSION_free(ex);
  }

  if (name) {
    name = NULL; // Don't think we need to free this.
  }

  return c;
}

Certificate *Certificate::fromString(std::string s) {
  const unsigned char *buff;
  const unsigned char **in;
  int ret = 0;
  buff = reinterpret_cast<const unsigned char *>(s.c_str());
  in = &buff;
  X509 *c509 = X509_new();
  HRNULL(c509);
  HRNULL(d2i_X509(&c509, in, s.length()));
 error_out:
  if (!ret) {
    return new Certificate(c509);
  } else {
    if (c509 != NULL) {
      X509_free(c509);
    }
    return NULL;
  }
}



}}} // namespace
