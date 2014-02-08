/* DBMoverData.h -*- c++ -*- 
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
#ifndef TEXTUS_MOVER_DBMOVERDATA_H_
#define TEXTUS_MOVER_DBMOVERDATA_H_

#include "textus/db/DBType.h"

namespace textus { namespace mover {

using namespace textus::db;

class DBMoverData: public DBType {
  BEGIN_DB_DECL(DBMoverData);
  DECL_DB_ENTRY(DBMoverData, string, DBString<512>, hash, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(DBMoverData, int, Number<2>, countdown, 0);
  END_DB_DECL(DBMoverData);
public:
  virtual string table(); // Default table for the type.
};

}} //namespace


#endif // TEXTUS_MOVER_DBMOVERDATA_H_
