/* EventQueue.h -*- c++ -*-
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

/*
 * Lock order is Event, then EventQueue, then event_list inside of the
 * EventQueue.
 */

#ifndef TEXTUS_EVENT_EVENTQUEUE_H
#define TEXTUS_EVENT_EVENTQUEUE_H
#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/locks/Mutex.h"

namespace textus { namespace event {

class Event;

class EventQueue: virtual public Base {
private:
  textus::base::ReferenceList<Event *> event_list;
  textus::base::logic::Bool should_quit;
  Condition condition;
  Mutex lock;

  static Duration time_on_queue;
  static int time_outs;


public:
  static EventQueue *preferredQueue();
  static void setPreferredQueue(EventQueue *eq);

  EventQueue():event_list(), should_quit(false), condition(), lock()
  {
  }

  virtual ~EventQueue()
  {
  }

  static EventQueue *defaultQueue();
  static EventQueue *failSafeQueue();

  void post(Event *e)
  {
    if (textus::base::init::shutdown_started) {
      return;
    }
    {
      LOCK(&lock);
      event_list.push_back(e);
    }
    condition.signal();
  }

  void remove(Event *e) {
    LOCK(&lock);
    event_list.erase(e);
  }


  void recordTimeOnQueue(const Duration &d)
  {
    time_on_queue += d;
  }

  void timeOut(Event *e)
  {
    ++time_outs;
    // Our reference is the one on the reference list.  When we erase
    // it, we would be sol with out this extra ref.
    e->ref();
    {
      LOCK(&lock);
      event_list.erase(e);
    }
    e->repost();
  }

  void processEvents();

  void waitForEvent()
  {
    LOCK(&lock);
    while (event_list.empty() && !should_quit) {
      condition.wait(lock);
    }
  }

  void threadLoop() {
    lock.lock();
    while (!should_quit) {
      lock.unlock();
      waitForEvent();
      processEvents();
      lock.lock();
    }
    lock.unlock();
  }

  void threadExit() {
    should_quit = true; // racy, but could get the message through quicker.
    lock.lock(); // get the lock.
    should_quit = true; // just in case someone else messed with it.
    lock.unlock();
    condition.signal();
  }
};

}} // namespace

#endif // TEXTUS_EVENT_EVENTQUEUE_H
