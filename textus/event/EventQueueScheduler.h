/* EventQueueScheduler.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Used to schedule one or more EventQueues from a single object/thread.
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

#ifndef TEXTUS_EVENT_EVENT_QUEUE_SCHEDULER_H_
#define TEXTUS_EVENT_EVENT_QUEUE_SCHEDULER_H_

#include "textus/base/Base.h"
#include "textus/event/EventQueue.h"

namespace textus { namespace event {

class EventQueueScheduler: public Base {
private:
  AutoDeref<EventQueue> eq;

protected:

public:
  EventQueueScheduler() {}
  virtual ~EventQueueScheduler() {}

  void setEventQueue(EventQueue *e) {
    Synchronized(this);
    eq = e;
  }

  EventQueue *eventQueue() {
    Synchronized(this);
    return eq;
  }

  virtual EventQueue *getQueue() {
    Synchronized(this);
    if (eq) {
      return eq;
    }

    return EventQueue::defaultQueue();
  }

  virtual void addQueue(EventQueue *e) {
    setEventQueue(e);
  }

  virtual void eraseQueue(EventQueue *e) {
    setEventQueue(NULL);
  }
  
};

}} //namespace

#endif // TEXTUS_EVENT_EVENT_QUEUE_SCHEDULER_H_
