/* MultiKeyDescription.h -*- c++ -*-
 * COpyright (c) 2014 Ross Biro
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

#ifndef TEXTUS_MOVER_MULTIKEYDESCRIPTION_H_
#define TEXTUS_MOVER_MULTIKEYDESCRIPTION_H_

#include <string>

#include "textus/base/Base.h"
#include "textus/mover/MoverEncryption.h"
#include "textus/mover/KeyDescription.h"
#include "textus/base/ReferenceValueMap.h"

namespace textus { namespace mover {
using namespace textus::base;
using namespace std;

class MultiKeyDescription: virtual public Base {
private:
  MVAR(public, MoverEncryption, encryption );
  // The keys must have the same encryption and must
  // be indistinguishable for the decrypt command.
  ReferenceValueMap<string, KeyDescription *> descriptions;
  
protected:
  bool isCompatible(KeyDescription *k1, KeyDescription *k2);

public:
  explicit MultiKeyDescription() {}
  virtual ~MultiKeyDescription() {}
  bool process(string data);
  bool addKeyDescription(KeyDescription *kd);
  
};

}}

#endif  //TEXTUS_MOVER_MULTIKEYDESCRIPTION_H_
