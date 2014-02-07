/* DBStatement.h -*- c++ -*-
 * Copyright (c) 2011 Ross Biro
 * Contains the base classes that represents a database statement.
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
