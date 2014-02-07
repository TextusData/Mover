/* DBManyMany.h -*- c++ -*-
 * Copyright (c) Ross Biro 2013
 *
 * Represents a many many relation table
 * Basically two UUID's.
 */

#ifndef TEXTUS_DB_DBMANYMANY_H
#define TEXTUS_DB_DBMANYMANY_H

#include "textus/db/DBType.h"
#include "textus/db/DBAutoDeref.h"


namespace textus { namespace db {

template <class A, class B> class DBManyMany: public DBType {
  BEGIN_DB_DECL(DBManyMany);
  DECL_DB_ENTRY(DBManyMany, UUID, DBAutoDeref<A>, key1, 0);
  DECL_DB_ENTRY(DBManyMany, UUID, DBAutoDeref<B>, key2, 0);
  END_DB_DECL(DBManyMany);

public:
  DBManyMany() {}
  virtual ~DBManyMany() {}

  virtual string table() { return ""; }
};

TEMPLATE_DB_DECL(DBManyMany, A,  B )
//template <class A, class B> DEFINE_TEMPLATE_DB_ENTRY(DBManyMany, UUID, DBAutoDeref<A>, key1, 0);
//template <class A, class B> DEFINE_TEMPLATE_DB_ENTRY(DBManyMany, UUID, DBAutoDeref<B>, key2, 0);
}} //namespace



#endif // TEXTUS_DB_DBMANYMANY_H
