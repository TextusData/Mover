/* EventQueue.cc -*- c++ -*-
 * Copyright (c) 2009, 2013 Ross Biro
 *
 * Queues up events with the fall back of reposting them if they are
 * not handled in a reasonable amount of time.
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

#include "textus/event/EventQueue.h"
#include "textus/base/ThreadLocal.h"
#include "textus/event/TimerQueue.h"
#include "textus/threads/WorkerThread.h"
#include "textus/base/init/Init.h"

namespace textus { namespace event {

int EventQueue::time_outs = 0;
Duration EventQueue::time_on_queue;
static ThreadLocal<EventQueue *> static_preferred_queues;
static EventQueue * failSafeEventQueue;
static Thread *failSafeThread;

EventQueue *EventQueue::preferredQueue() {
  return static_cast<EventQueue *>(static_preferred_queues);
}

void EventQueue::setPreferredQueue(EventQueue *eq) {
  if (preferredQueue()) {
    preferredQueue()->deref();
  }
  static_preferred_queues = eq;
  if (eq) {
    eq->ref();
  }
}

EventQueue *EventQueue::defaultQueue() {
  return textus::threads::WorkerThread::workerThreadScheduler()->getQueue();
}

EventQueue *EventQueue::failSafeQueue() {
  return failSafeEventQueue;
}

void EventQueue::processEvents()
{
  lock.lock();
  while (!should_quit && !event_list.empty()) {
    {
      AUTODEREF(Event *, ev);
      ev = event_list.front();
      Synchronized(ev);
      ev->ref(); // Popping the event will deref it.
      event_list.pop_front();
      lock.unlock();

      TimerQueue::defaultTimerQueue()->removeTimer(ev);

      ev->current_queue->weakDeref();
      ev->current_queue = NULL;

      if (ev != NULL && !textus::base::init::shutdown_started) {
	ev->do_it();
      }
    }
    lock.lock();
  }
  lock.unlock();
}

static Base *failSafeThreadStart(Base *b)
{
  failSafeThread = Thread::self();
  LOG(DEBUG) << "failSafeThread::" << failSafeThread << "\n";
  failSafeThread->ref();
  failSafeEventQueue = new EventQueue();
  failSafeEventQueue->threadLoop();
  failSafeEventQueue->deref();
  failSafeEventQueue = NULL;
  return NULL;
}

static void failSafeThreadStop() {
  if (failSafeEventQueue) {
    failSafeEventQueue->threadExit();
  }
  if (failSafeThread) {
    failSafeThread->join();
    failSafeThread->deref();
  }
}

INITTHREAD(failSafeThreadStart, NULL);
SHUTDOWNTHREAD(failSafeThreadStop);

}} // namespace
