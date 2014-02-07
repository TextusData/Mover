/* DBMoverData.h -*- c++ -*- 
 * Copyright (c) 2013 Ross Biro
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
