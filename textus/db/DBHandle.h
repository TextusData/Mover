/* DBHandle.h -*- c++ -*-
 * Copyright (c) 2010-2012 Ross Biro
 * Contains the classes that allow us to communicate with a db.
 */

#ifndef TEXTUS_DB_DBHANDLE_H_
#define TEXTUS_DB_DBHANDLE_H_

#include "textus/base/Base.h"
#include "textus/base/Password.h"
#include "textus/db/DBPool.h"
#include "textus/db/DBStatement.h"

#include "string"

namespace textus { namespace db {

#include "sql.h"
#include "sqltypes.h"
#include "sqlext.h"

using namespace textus::base;
using namespace std;

class DBPool;
class DBHandle;
class DBStatement;

class DBInstance: virtual public Base {
private:
  string serverName;
  string userName;
  Password passwrd;

  WMVAR(public, DBPool, pool);

  AutoDeref<DBHandle> handle;

protected:
  virtual int weakDelete();

public:
  DBInstance() {}
  virtual ~DBInstance() {}

  static DBInstance *createFromDSN(std::string dsn) {
    int ret=0;
    DBInstance *i = new DBInstance();
    HRNULL(i);
    i->setServer(dsn.c_str());
    HRC(i->connect());
    HRNULL(i->getHandle());
    return i;

  error_out:
    if (i != NULL) {
      i->deref();
    }
    return NULL;
  }
  
  void setServer(const string s) {
    serverName = s;
  }

  string server() const {
    return serverName;
  }

  string user() const {
    return userName;
  }

  Password password() const {
    return passwrd;
  }

  void setUser(const string s) {
    userName = s;
  }

  void setPassword(const Password pass) {
    passwrd = pass;
  }

  void setPool(DBPool *p) { return setpool(p); }

  bool pending() { return false; }
  void needSync() {}

  int connect(); // connect to the instance.

  DBHandle *getHandle(); // returns a handle to this instance.
  void returnHandle(DBHandle *h);
  
  DBHandle *writeHandle() { return getHandle(); }

};

class DBHandle: virtual public Base {
private:
  SQLHENV env;
  SQLHDBC conn;
  WMVAR(public, DBInstance, instance);

protected:
  virtual int weakDelete() {
    setinstance(NULL);
    return Base::weakDelete();
  }

public:
  static DBHandle *defaultHandle();
  DBHandle(): env(NULL), conn(NULL) {}
  virtual ~DBHandle() { close(); }

  virtual DBStatement *prepare(string sql);
  virtual bool isReadHandle() { 
    return true;
  }

  DBPool *pool() { return instance()->pool(); }
  int connect(const DBInstance &);
  int driverConnect(const DBInstance &);
  int close();
  int execute(string sql);

};

}} //namespace

#endif // TEXTUS_DB_DBHANDLE_H_
