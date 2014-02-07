/* RoundRobinEventQueueScheduler.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Used to schedule one or more EventQueues from a single object/thread.
 *
 */

#ifndef TEXTUS_EVENT_ROUND_ROBIN_EVENT_QUEUE_SCHEDULER_H_
#define TEXTUS_EVENT_ROUND_ROBIN_EVENT_QUEUE_SCHEDULER_H_

#include "textus/event/EventQueueScheduler.h"
#include "textus/base/ReferenceList.h"

namespace textus { namespace event {

class RoundRobinEventQueueScheduler: public EventQueueScheduler {
private:
  ReferenceList<EventQueue *> queues;

protected:
public:
  RoundRobinEventQueueScheduler() {}
  virtual ~RoundRobinEventQueueScheduler() {}

  virtual EventQueue *getQueue() {
    Synchronized(this);
    if (queues.empty()) {
      return EventQueueScheduler::getQueue();
    }
    EventQueue *e = queues.front();
    queues.pop_front();
    queues.push_back(e);
    return e;
  }

  void addQueue(EventQueue *eq) {
    Synchronized(this);
    queues.push_front(eq);
  }

  void eraseQueue(EventQueue *eq) {
    Synchronized(this);
    queues.erase(eq);
  }

  void shutdown(void) {
    Synchronized(this);
    while (!queues.empty()) {
      EventQueue *eq = queues.front();
      eq->threadExit();
      queues.pop_front();
    }
  }
  
};

}} //namespace

#endif // TEXTUS_EVENT_ROUND_ROBIN_EVENT_QUEUE_SCHEDULER_H_
