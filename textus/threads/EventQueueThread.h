/* EventQueueThread.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Handle eventQueue threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one eventQueue thread per cpu is likely ideal.
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
#include "textus/event/EventQueueScheduler.h"
#include "textus/base/Base.h"

#ifndef TEXTUS_THREADS_EVENTQUEUE_THREAD_H_
#define TEXTUS_THREADS_EVENTQUEUE_THREAD_H_

namespace textus { namespace threads {
using namespace textus::base;

class EventQueueThread: public Thread {
private:
  AutoDeref<textus::event::EventQueue> eventQueue;
  AutoDeref<textus::event::EventQueueScheduler> scheduler;

protected:
  static Base *threadStartup(Base *b);

public:
  explicit EventQueueThread(textus::event::EventQueueScheduler *ts): Thread(NULL, NULL), scheduler(ts)
  {
    AUTODEREF(textus::event::EventQueue *, eq);
    start_up = threadStartup;
    // XXXXX FIXME:  It looks like a memory leak here.
    eq = new textus::event::EventQueue();
    eventQueue = eq;
    // argument eventually gets dereffed, so we
    // don't have to worry about the extra ref.
    argument = static_cast<Base *>(eventQueue);
    scheduler->addQueue(eventQueue);
    unblock();
  }

  virtual ~EventQueueThread() {
    scheduler->eraseQueue(eventQueue);
    eventQueue = NULL;
  }
};

}} //namespace


#endif //TEXTUS_THREADS_EVENTQUEUE_THREAD_H_
