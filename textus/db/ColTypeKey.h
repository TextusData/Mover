/* ColTypeKey.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Encapsulate a key in the database.
 * We'll use a string so that 
 * people may be able to identify and extract it.
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

#ifndef TEXTUS_DB_COLTYPEKEY_H_
#define TEXTUS_DB_COLTYPEKEY_H_

#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/keys/Key.h"

namespace textus { namespace db {

using namespace textus::keys;

class ColTypeKey: public ColLobBase<SQL_VARCHAR> {
public:
  ColTypeKey() {}
  virtual ~ColTypeKey() {}
  
  Key *value() const {
    return Key::fromString(copyOut().c_str());
  }

  void setValue(Key *k) {
    copyIn(k->toString());
  }

  string sqlType() {
    string s = "varchar";
    return s;
  }

};

}} //namespace


#endif //TEXTUS_DB_COLTYPEKEY_H_
