/* SSLVerifier.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Base class for things which decide which certificate chains to accept.
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

