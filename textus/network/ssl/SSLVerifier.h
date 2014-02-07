/* SSLVerifier.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Base class for things which decide which certificate chains to accept.
 *
 */

#include "textus/network/ssl/SecureSocket.h"

#ifndef TEXTUS_NETWORK_SSL_VERIFIER_H_
#define TEXTUS_NETWORK_SSL_VERIFIER_H_

namespace textus { namespace network { namespace ssl {

class SSLVerifier: virtual public textus::base::Base {
private:

public:
  SSLVerifier() {}
  virtual ~SSLVerifier() {}
  virtual int verify(class SecureSocket *, int preverify_ok, X509_STORE_CTX *) = 0;
};


}}} //namespace

#endif // TEXTUS_NETWORK_SSL_VERIFIER_H_

