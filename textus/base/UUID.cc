/* UUID.h -*- C++ -*-
 * Copyright (C)  2013 Ross Biro
 *
 * A class to represent a UUID.
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


#include "UUID.h"

namespace textus { namespace base {

std::ostream& operator<< (std::ostream& o, const UUID & uuid)
{
  return o << uuid.toString();
}

std::ostream& operator<< (std::ostream& o, const UUID * const uuid)
{
  return o << uuid->toString();
}

}} //namespace
