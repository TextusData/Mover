/* DBMoverData.cc -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
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

#include "textus/mover/DBMoverData.h"
#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"

namespace textus { namespace mover {

STATIC_DB_DECL(DBMoverData);

string DBMoverData::table() {
  return string("mover_hashes");
}

}} //namespace
