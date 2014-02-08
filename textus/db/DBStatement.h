/* DBStatement.h -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 * Contains the base classes that represents a database statement.
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

// We must incolude ColType.h first.  
#include "textus/db/ColType.h"

#ifndef TEXTUS_DB_DBSTATEMENT_H_
#define TEXTUS_DB_DBSTATEMENT_H_

#include "textus/base/Base.h"
#include "textus/base/ReferenceList.h"
#include "textus/db/DBBase.h"
#include "textus/db/DBType.h"

namespace textus { namespace db {

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>

using namespace textus::base;

class DBType;
class ColBase;


class DBStatement: virtual public Base {
private:
  friend class DBHandle;
  SQLHSTMT handle;
  
public:
  explicit DBStatement() {}
  virtual ~DBStatement() {}

  SQLHSTMT hstmt() { return handle; }
  virtual int execute();
  virtual int bindParams(ReferenceList<ColBase *> *p);
  virtual int bindParams(DBType **, size_t);

  // Already bound.  Need to set the row count for block fetches.
  int fetchData(DBType *result, size_t &count /* in/out */);
  int fetchDataByColumn(size_t &count);  /* used for pass 2 for blobs et al. */
  virtual int readonly();
  virtual int bindColNull(int colnum);
  virtual int unbindColumns();
};

}} // namespace

#endif //TEXTUS_DB_DBSTATEMENT_H_
