/* DBEvent.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * Base class for all database events.
 *
 */

#ifndef TEXTUS_DB_DBEVENT_H_
#define TEXTUS_DB_DBEVENT_H_

#include "textus/event/Event.h"

namespace textus { namespace db {

class DBEvent: public textus::event::Event {
private:
  MVAR(public, DBInstance, instance);
  string sql;
  MVAR(public, ReferenceList<ColBase *>, parm);
  DBType **parms;
  size_t parms_count;
  MVAR(public, DBPool, pool);
  int *ordering;

public:
  virtual void do_it() {
    int ret=0;
    AUTODEREF(DBStatement *, stmt);
    HRNULL(stmt = instance()->getHandle()->prepare(sql));
    if (parm() && parm()->size()) {
      // single set of parameters
      HRC(stmt->bindParams(parm()));
    } else if (parms_count > 0) {
      // parameters stuck in a dbtype.  Usually runs the whole thing over and over.
      HRC((*parms)->bindParams(stmt, dbUpdate, ordering));
    }
    HRC(stmt->execute());
    pool()->returnInstance(instance());
    return;
  error_out:
    pool()->invalidateInstance(instance());
    return;
  }

public:
  explicit DBEvent(textus::event::EventTarget *t): Event(t), parms(NULL), parms_count(0), ordering(NULL) {}
  virtual ~DBEvent() {}
  void setSQL(string s) {
    Synchronized(this);
    sql = s;
  }

  string getSQL() {
    Synchronized(this);
    return sql;
  }

  void setparms(DBType **p) { parms = p; }
  void setparms_count(size_t s) { parms_count = s; }
  DBType **getparms() { return parms; }
  size_t getparms_count() { return parms_count; }

};

}} //namespace

#endif //TEXTUS_DB_DBEVENT_H_
