/* MoverVerifier.h -*- c++ -*- 
 * Copyright (c) 2013 Ross Biro
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
