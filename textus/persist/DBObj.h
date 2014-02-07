/* DBObj.h -*- C++ -*- 
 * Copyright (C) 2010 Ross Biro
 * All persistant DB objects should inherit from this class.
 *
 */

#ifndef TEXTUS_PERSIST_DB_DBOBJ_H_
#define TEXTUS_PERSIST_DB_DBOBJ_H_

namespace textus { namespace persist { namespace db {

/*! \class DBObj DBObj represents the base class from which all
 * persistant objects that may be stored in a database should be
 * derived.
 *
 * Key things: It needs to know a default table to store the class in,
 * and all the other objects that need to be grouped into a single
 * transaction.  It also needs to know about any dependencies that
 * don't necessarily need to be updated in a single transaction.
 */

class DBObject: virtual public Base {
private:
  AutoDeref<class DBHandle> db; // database to store this thing in.

public:
  explicit DBObject() {}
  virtual ~DBObject() {}
  
};

}}} //namespace

#endif // TEXTUS_PERSIST_DB_DBOBJ_H_
