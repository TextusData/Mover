/* DBInstanceInfo.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * information about all the instances in the current pool
 * so that we connect to all of them.
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

#ifndef TEXTUS_DB_DBINSTANCE_INFO_H_
#define TEXTUS_DB_DBINSTANCE_INFO_H_

#include "textus/db/DBType.h"

namespace textus { namespace db {

class DBInstanceInfo: public DBType {
private:
  BEGIN_DB_DECL(DBInstanceInfo);
  DECL_DB_ENTRY(DBInstanceInfo, UUInfo, ColTypeUUInfo, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(DBInstanceInfo, string, DBString<128>, host, 0);
  DECL_DB_ENTRY(DBInstanceInfo, int, ColTypeInt, port, 0);
  DECL_DB_ENTRY(DBInstanceInfo, string, DBString<128>, name, 0);
  DECL_DB_ENTRY(DBInstanceInfo, string, DBString<128>, user, 0);
  END_DB_DECL(DBInstanceInfo);
};

}} // namespace

#endif
