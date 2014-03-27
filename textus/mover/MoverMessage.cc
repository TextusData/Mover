/* MoverMessage.cc -*- c++ -*-
 * Copyright (c) Ross Biro
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

#include <sstream>

#include "textus/base/init/Init.h"
#include "textus/mover/Mover.h"
#include "textus/mover/MoverMessage.h"
#include "textus/mover/MoverMessageProcessor.h"

namespace textus { namespace mover {

using namespace std;

DEFINE_INT_ARG(mover_random_data_max, 16384, "mover_random_data_max", "The maximum size of the random chunk that is incl*uded in every message.");
DEFINE_INT_ARG(mover_random_data_min, 4096, "mover_random_data_min", "The minimum size of the random chunk that is included in every message.");

string MoverMessage::toString() const {
  stringstream ss;
  ss << "MoverMessage: type = " << (int)messageType() 
     << ", length = " << data.length();
  return ss.str();
}

}} //namespace

