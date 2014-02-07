/* DBTypeUUID.h -*- c++ -*-
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

class DBTypeUUID: virtual public DBType {
private:
  BEGIN_DB_DECL(DBTypeUUID);
  DECL_DB_ENTRY(DBTypeUUID, UUID, ColTypeUUID, key, 0);
  END_DB_DECL(DBTypeUUID);

public:
  DBTypeUUID() {}
  virtual ~DBTypeUUID() {}
  virtual std::string table() { return std::string(""); }
  
};


}} //namespace

#endif //TEXTUS_DB_DBTYPEUUID_H_
