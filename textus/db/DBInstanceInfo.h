/* DBInstanceInfo.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * information about all the instances in the current pool
 * so that we connect to all of them.
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
