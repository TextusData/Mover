/* DBPool.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * Represents a pool of databases that should
 * all be equivalent.
 *
 */

#ifndef TEXTUS_DB_DBPOOL_H_
#define TEXTUS_DB_DBPOOL_H_

#include "textus/base/Base.h"
#include "textus/db/DBBase.h"
#include "textus/db/DBHandle.h"
#include "textus/db/DBType.h"
#include "textus/base/ReferenceSet.h"
#include "textus/base/Queue.h"
#include "textus/base/locks/ReadWriteLock.h"
#include "textus/event/EventWatcher.h"

namespace textus { namespace db {

using namespace textus::base;
using namespace std;

class DBInstance;
class DBHandle;
class DBType;
class ColBase;

class DBPool: public textus::event::EventTarget {
private:
  string name;
  typedef ReferenceSet<DBInstance *> InstanceSet;
  typedef Queue<DBInstance *> InstanceQueue;
  
  bool writing;
  InstanceSet instances;
  InstanceQueue queue;
  ReadWriteLock rwlock; // Goes before synchronized on the pool.
  bool going;
  static std::string (*default_dsn_func)();
  static bool staticGoing;
  static bool staticPoolGoing() {
    return staticGoing;
  }

#ifdef TESTING
  friend class textus::testing::UnitTest;
#endif

protected:
  static string defaultDSN();
  DBHandle *writeHandle();
  DBHandle *getReadHandle();

  void setWriting() {
    Synchronized(this);
    writing = true;
  }

  void clearWriting() {
    Synchronized(this);
    writing = false;
  }

  bool isWriting() {
    Synchronized(this);
    return writing;
  }

  static void staticReturnHandle(DBHandle *);

  unsigned minSafeWrites() { return 3; }

public:
  static DBPool *getPool();     // returns the static pool.
  static void cleanup();        // cleans up the static pool.

  explicit DBPool(): going(false) {
    // start off readonly so that we
    // can get all the instances loaded
    // before we start writing and get
    // them out of sync.
    rwlock.readLock();
  }

  virtual ~DBPool() {
    queue.clear();
    instances.clear();
  }

  void returnInstance(DBInstance *i) {
    Synchronized(this);
    queue.add(i);
    if (queue.size() > minSafeWrites()) {
      clearWriting();
    }
  }


  bool writeInProgress() {
    return rwlock.writeLocked();
  }

  /* only call when something has gone horribly wrong. */
  void clearInstances() {
    going = false;
    queue.clear();
    instances.clear();
  }

  void returnHandle(DBHandle *h);
  void invalidateHandle(DBHandle *h);
  void invalidateInstance(DBInstance *i);

  /* only call this once per object. */
  void go() {
    assert(!going);
    going = true;
    rwlock.readUnlock();
  }

  bool isGoing() const {
    return going;
  }
  
  int addInstance(DBInstance *i);

  int removeInstance(DBInstance *i) {
    Synchronized(this);
    queue.erase(i);
    return instances.erase(i);
  }

  /* Must only read. */
  int query(string sql, size_t maxrows, DBType *result, ColBase *param);
  int query(string sql, size_t maxrows, DBType *result, ReferenceList<ColBase *> *params=NULL);

  /* Execute is assumed to write. */
  int execute(string sql, ReferenceList<ColBase *> *params=NULL);
  int execute(string sql, ColBase *param);

};

typedef AutoFoo<DBHandle, DBPool::staticReturnHandle> AutoReturn;


}} // namespace

#endif // TEXTUS_DB_DBPOOL_H_
