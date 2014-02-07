/* DBName.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 * Contains the base classes that represents a uuid as stored in a database.
 * We are storing UUID's as strings to that they might be human readable.
 *
 */

#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"

#ifndef TEXTUS_DB_DBTYPEUUID_H_
#define TEXTUS_DB_DBTYPEUUID_H_

namespace textus { namespace db {

class DBName: virtual public DBType {
private:
  BEGIN_DB_DECL(DBName);
  DECL_DB_ENTRY(DBName, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(DBName, std::string, DBString<256>, name, 0);
  END_DB_DECL(DBName);

public:
  DBName() {}
  virtual ~DBName() {}
  virtual std::string table() { return std::string(""); }
  
};


}} //namespace

#endif //TEXTUS_DB_DBTYPEUUID_H_
