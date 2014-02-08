/* ColTypeUUID.h -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 * Contains the base classes that represents a uuid as stored in a database.
 * We are storing UUID's as strings to that they might be human readable.
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

#include "textus/base/UUID.h"
#include "textus/db/ColType.h"

#ifndef TEXTUS_DB_COLTYPEUUID_H_
#define TEXTUS_DB_COLTYPEUUID_H_

namespace textus { namespace db {

using namespace textus::base;

class ColTypeUUID: public DBStringBase<36> {
public:
  ColTypeUUID() {}
  virtual ~ColTypeUUID() {}

  UUID value() const {
    UUID uu(copyOut());
    return uu;
  }

  void setValue(const UUID u) {
    copyIn(u.toString());
  }

  void generate() {
    UUID uu;
    uu.generate();
    setValue(uu);
  }
};

}} // namespace

#endif
