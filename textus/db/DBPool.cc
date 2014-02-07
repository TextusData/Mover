/* DBPool.cc -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 *
 */
#include "textus/base/Base.h"
#include "textus/db/DBType.h"
#include "textus/db/DBPool.h"
#include "textus/db/DBInstanceID.h"
#include "textus/db/DBEvent.h"
#include "textus/db/DBThread.h"

#define MAX_POOL_DBS 100

namespace textus { namespace db {

DEFINE_STRING_ARG(db_pool_default_driver, "PostgreSQL", "db_pool_default_driver", "ODBC Driver to use by default for database pools.  Defaults to PostgreSQL.");
DEFINE_STRING_ARG(db_pool_default_server, "dbserver.cluster", "db_pool_default_server", "Hostname for the database server to use.  Defaults to dbserver.cluster");
DEFINE_STRING_ARG(db_pool_default_database, "data", "db_pool_default_database", "Database to connect to by default.  Defaults to data.");
DEFINE_INT_ARG(db_pool_default_port, 5432, "db_pool_default_port", "Port to connect to by default. Defaults to 5432.");
DEFINE_STRING_ARG(db_pool_default_user, "dbuser", "db_pool_default_user", "User name to use to connect to the database.");

int DBPool::addInstance(DBInstance *i) {
  Synchronized(this);

  pair<InstanceSet::iterator, bool> p = instances.insert(i);

  if (!p.second) {
    LOG(WARNING) << "Tried to add db instance to pool more than once." << "\n";
    return 1;
  }

  Synchronized(i);
  i->setPool(this);
  if (!i->pending()) { 
    // This really shouldn't happen, except at startup.  And then we will hold the write lock
    // until the pool has initialized to prevent partial writes.
    // Give the new guy a second.
    if (writeInProgress()) {
      i->needSync(); // let the instance know it may have missed a write.
    }
    queue.add(i);
  }
  return 0;
}

DBHandle *DBPool::getReadHandle() {
  rwlock.readLock();
  Synchronized(this);
  DBInstance *i = queue.get();
  if (i == NULL) {
    return NULL;
  }
  return i->getHandle();
}

void DBPool::returnHandle(DBHandle *h) {
  Synchronized(this);
  DBInstance *i = h->instance();
  assert(i != NULL);
  queue.add(i);
  rwlock.readUnlock();
}

void DBPool::invalidateHandle(DBHandle *h) {
  DBInstance *i = h->instance();
  assert(i != NULL);
  invalidateInstance(i);
  rwlock.readUnlock();
}

void DBPool::invalidateInstance(DBInstance *i) {
  i->needSync();
}

bool DBPool::staticGoing = false;

DBPool *DBPool::getPool() {
  int ret = 0;
  static DBPool *pool=new DBPool();
  Synchronized(pool);
  if (!pool->isGoing()) {
    AUTODEREF(DBInstance *, instance);
    AUTODEREF(DBInstanceID *, ids);    
    instance = new DBInstance();
    HRNULL(instance);
    instance->setServer(defaultDSN());
    instance->setUser(db_pool_default_user);
    HRC(instance->connect());
    HRNULL(instance->getHandle());
    ids = DBInstanceID::allocResultSet(MAX_POOL_DBS);
    HRNULL(ids);
    // XXXXX Fixme:  Need to set this up to handle unlimited instances and split it off so
    // other pool creation functions can use it.
    ret = DBInstanceID::query(string("select pk, name, dsn from dbinstances"), MAX_POOL_DBS, ids, instance->getHandle());
    if (ret < 1) {
      goto error_out;
    }
    for (int i = 0; i < ret; ++i) {
      string dsn = ids[i].dsn();
      AUTODEREF(DBInstance *, i2);
      i2 = DBInstance::createFromDSN(dsn);
      if (i2 == NULL) {
	continue;
      }
      pool->addInstance(i2);
    }
    pool->go();
    staticGoing = true;
  }
  pool->ref();
  return pool;
 error_out:
  pool->clearInstances();
  return NULL;
}

void DBPool::staticReturnHandle(DBHandle *h) {
  h->pool()->returnHandle(h);
}

static std::string localDefaultDSN() {
  return string("Driver={") + db_pool_default_driver + string("};Server=") + db_pool_default_server +
    string(";Port=") + to_string(db_pool_default_port) + string(";Database=") + db_pool_default_database + string(";");
}

std::string (*DBPool::default_dsn_func)() = localDefaultDSN;

string DBPool::defaultDSN() {
  return default_dsn_func();
}

int DBPool::query(string sql, size_t maxrows, DBType *result, ColBase *param) {
  ReferenceList<ColBase *> r;
  r.push_back(param);
  return query(sql, maxrows, result, &r);
}

int DBPool::query(string sql, size_t maxrows, DBType *result, ReferenceList<ColBase *> *params) {
  int ret=0;
  do {
    DBHandle *h = getReadHandle();
    ret = result->query(sql, maxrows, result, h, params);
    if (ret < 0) {
      invalidateHandle(h);
    } else {
      returnHandle(h);
    }
  } while (ret < 0);
  return ret;
}

  /* Execute is assumed to write.  Most commonly used for update. */
int DBPool::execute(string sql, ColBase *param) {
  ReferenceList<ColBase *> p;
  p.push_back(param);
  return execute(sql, &p);
}

int DBPool::execute(string sql, ReferenceList<ColBase *> *params) {
  int ret = 0;
  rwlock.writeLock();
  Synchronized(this);
  queue.clear(); 
  setWriting();
  foreach (i, instances) {
    AUTODEREF(DBEvent *, event);
    // wait until we get the lock and stop all the reads.  This could be a bit.
    event = new DBEvent(this);
    HRNULL(event);
    event->setSQL(sql);
    event->setpool(this);
    event->setinstance(*i);
    event->setparm(params);
    DBThread::dbThreadScheduler()->eventQueue()->post(event);
  }

  // We need to block until at least a certain number of writes have completed.
  while (isWriting()) {
    wait();
  }
 error_out:
  rwlock.writeUnlock();
  return ret;
}

void DBPool::cleanup() {
  if (staticPoolGoing()) {
    DBPool *p = getPool();
    Synchronized(p);
    p->going=false;
    p->instances.clear();
    p->queue.clear();
    staticGoing = false;
  }
}


}} //namespace
