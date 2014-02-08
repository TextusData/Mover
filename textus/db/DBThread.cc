/* DBThread.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Handle DB worker threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.
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

#include "textus/db/DBThread.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace db {

using namespace textus::base;

textus::event::RoundRobinEventQueueScheduler *DBThread::dbThreadScheduler() {
  static textus::event::RoundRobinEventQueueScheduler rres;
  return &rres;
}

DEFINE_INT_ARG(db_thread_count, 5, "db_thread_count", "Number of db db to fire up.  Defaults to 5.");
static Thread **db_thread_array;

DEFINE_INIT_FUNCTION(dbThreadStartup, AUXILLARY_THREAD_INIT_PRIORITY + 1) {

  db_thread_array = static_cast<Thread **>(malloc(sizeof(*db_thread_array)*db_thread_count));

  for (int i=0; i < db_thread_count; i++) {
    // dereffed when the thread exits.
    db_thread_array[i] = new DBThread();
  }

  return 0;
}

DEFINE_SHUTDOWN_FUNCTION(dbThreadShutdown, AUXILLARY_THREAD_INIT_PRIORITY + 1) {
  DBThread::dbThreadScheduler()->shutdown();
  for (int i = 0; i < db_thread_count; i++) {
    db_thread_array[i]->join();
    db_thread_array[i]->deref();
  }
  free (db_thread_array);
}

}} //namespace
