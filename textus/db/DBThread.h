/* DBThread.h -*- C++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * DB Worker Threads.
 * Used for Asynchronous things like writes.  We write to a bunch of db's at once,
 * and start using them for reads again as soon as enough rights finish.
 *
 */

/*
 *   This program is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 3 of the
 *   License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see
 *   <http://www.gnu.org/licenses/>.
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
