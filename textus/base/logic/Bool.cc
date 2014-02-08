/* Bool.h  -*- c++ -*-
 * A boolean class with some error protection.
 *
 * Copyright (c) 2009 Ross Biro
 * Confidential Source Code 
 * May contain trade secrets.
 * Do Not Divuldge.
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


#include "textus/base/logic/Bool.h"
#include "textus/logging/Log.h"

namespace textus { namespace base { namespace logic {

static int CorruptBools = 0;

bool Bool::correct()
{
  if (value != BFalse && value != BTrue) {

    LOG(WARNING) << "Corrupt Boolean value: " << value << "\n";
    LOG_CALL_STACK(WARNING);
    CorruptBools++;

    int f = __builtin_popcount(value & BFalse);
    int t = __builtin_popcount(value & BTrue);

    if (f > t) {
      value = BFalse;
    } else if (f != t) {
      value = BTrue;
    } else {
      LOG(ERROR) << "Uncorrectable boolean: Assuming False\n";
      die();
    }
  }
  return (value == BTrue);
}

}}} //namespace
