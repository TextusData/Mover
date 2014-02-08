/* DBInstanceID.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * Represents a pool of databases that should
 * all be equivalent.
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

#ifndef TEXTUS_DB_DBINSTANCE_ID_H_
#define TEXTUS_DB_DBINSTANCE_ID_H_

#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"
#include "textus/db/DBAutoDeref.h"

namespace textus { namespace db {

class DBInstanceID: public DBType {
private:
  BEGIN_DB_DECL(DBInstanceID);
  DECL_DB_ENTRY(DBInstanceID, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(DBInstanceID, string, DBString<128>, name, 0);
  DECL_DB_ENTRY(DBInstanceID, string, DBString<256>, dsn, 0);
  //  DECL_DB_ENTRY(DBInstanceID, UUID, DBAutoDeref<DBInstanceInfo>, info, 0);
  END_DB_DECL(DBInstanceID);
public:
  std::string table() { return string("dbinstances"); }
};

}} // namespace

#endif
