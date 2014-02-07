/* DBCompositHandle.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Contains the classes that allow us to communicate with multiple db's at once.
 *
 */

#ifndef TEXTUS_DB_DBCOMPOSITHANDLE_H_
#define TEXTUS_DB_DBCOMPOSITHANDLE_H_

namespace textus { namespace db {

class DBCompositHandle: public DBHandle {
private:
  ReferenceList<DBHandle *> handles;

public:
  DBCompositHandle() {};
  virtual ~DBCompositHandle() {}
  
  
  virtual bool isWriteHandle() {
    if (handles.empty()) {
      return false;
    } else {
      handles.first()->isWriteHandle();
    }
  }

  virtual DBStatement *prepare(string sql) {
    DBCompositeStatement *stmt = new DBCompositeStatement();
  }

};

}} //namespace


#endif // TEXTUS_DB_DBCOMPOSITHANDLE_H_
