/* MoverVerifier.h -*- c++ -*- 
 * Copyright (c) 2013 Ross Biro
 *
 */

#ifndef TEXTUS_MOVER_MOVERVERIFIER_H
#define TEXTUS_MOVER_MOVERVERIFIER_H

#include "textus/base/Base.h"
#include "textus/network/ssl/SecureSocket.h"
#include "textus/network/ssl/SSLVerifier.h"

namespace textus { namespace mover {

using namespace textus::network::ssl;

class MoverVerifier: public SSLVerifier {
 public:
  explicit MoverVerifier() {}
  virtual ~MoverVerifier() {}
  virtual int verify(class SecureSocket *, int preverify_ok, X509_STORE_CTX *) {
    return 1;
  }
};

}}

#endif //TEXTUS_MOVER_MOVERVERIFIER
