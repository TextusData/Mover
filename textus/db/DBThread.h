/* DBThread.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * DB Worker Threads.
 * Used for Asynchronous things like writes.  We write to a bunch of db's at once,
 * and start using them for reads again as soon as enough rights finish.
 *
 */

#ifndef TEXTUS_DB_DBTHREAD_H_
#define TEXTUS_DB_DBTHREAD_H_

#include "textus/threads/EventQueueThread.h"
#include "textus/event/RoundRobinEventQueueScheduler.h"

namespace textus { namespace db {

class DBThread: public textus::threads::EventQueueThread {
private:
protected:
public:
  static textus::event::RoundRobinEventQueueScheduler *dbThreadScheduler();

  explicit DBThread(): EventQueueThread(dbThreadScheduler()) {}
  virtual ~DBThread() { }
};

}} //namespace

#endif // TEXTUS_DB_DBEVENT_H_
