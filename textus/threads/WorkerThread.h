/* WorkerThread.h -*- c++ -*-
 * Copyright (c) 2010-2012, 2014 Ross Biro
 *
 * Handle worker threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one worker thread per cpu is likely ideal.
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

#include "textus/base/Thread.h"
#include "textus/event/EventQueue.h"
#include "textus/event/RoundRobinEventQueueScheduler.h"
#include "textus/threads/EventQueueThread.h"
#include "textus/base/Base.h"

#ifndef TEXTUS_THREADS_WORKER_THREAD_H_
#define TEXTUS_THREADS_WORKER_THREAD_H_

namespace textus { namespace threads {
using namespace textus::base;

class WorkerThread: public EventQueueThread {
private:

protected:

public:
  static textus::event::RoundRobinEventQueueScheduler *workerThreadScheduler();
  static Thread *getFirst();

  WorkerThread(): EventQueueThread(workerThreadScheduler()) { }
  virtual ~WorkerThread() { }

};

}} //namespace


#endif //TEXTUS_THREADS_WORKER_THREAD_H_
