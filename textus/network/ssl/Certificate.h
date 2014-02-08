/* Certificate.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Represents a certificate, really a certificate chain.
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

#ifndef TEXTUS_NETWORK_SSL_CERTIFICATE_H_
#define TEXTUS_NETWORK_SSL_CERTIFICATE_H_

#include "textus/base/Base.h"
#include "textus/base/AutoFoo.h"

namespace textus { namespace network { namespace ssl {
extern "C" {
#include "openssl/ssl.h"
  extern void ERR_print_errors_fp(FILE *);
}

class Certificate: public virtual textus::base::Base {
private:
  static void X509_release(X509 *);
  static void RSA_release(RSA *);
  textus::base::AutoFoo<X509, X509_release> cert509;
  textus::base::AutoFoo<RSA, RSA_release> rsa;
  EVP_PKEY * private_key;

protected:
  void setPrivateKey(EVP_PKEY *pk) {
    Synchronized(this);
    private_key = pk;
  }

  void setRSA(RSA *r) {
    rsa = r;
  }

public:
  static Certificate *fromString(std::string s);
  static Certificate *fromFile(std::string fn);
  static Certificate *random(int bits=4096, int expires=60*60*24); // generates a random self-signed certificate.
  explicit Certificate(): cert509(NULL), private_key(NULL) {}
  explicit Certificate(X509 *c): cert509(c), private_key(NULL) {}
  virtual ~Certificate() {}

  X509 *x509() {
    Synchronized(this);
    return cert509;
  }

  EVP_PKEY *privateKey() {
    return private_key;
  }

  bool compatible(const Certificate *c) {
    return false;
  }

};

}}} //namespace


#endif //TEXTUS_NETWORK_SSL_CERTIFICATE_H_
