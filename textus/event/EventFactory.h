/* EventFactory.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Builds events.  Need this class to make sure all related events end
 * up int he same queue.
 */

#ifndef TEXTUS_EVENT_EVENT_FACTORY_H_
#define TEXTUS_EVENT_EVENT_FACTORY_H_

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/event/EventQueue.h"

namespace textus { namespace event {

class EventFactory: virtual public Base {
private:
  AutoDeref<EventQueue> q;

public:
  EventFactory() {}
  virtual ~EventFactory() {}

  EventQueue *getQueue() {
    Synchronized(this);
    if (q == NULL) {
      q = EventQueue::defaultQueue();
    }
    return q;
  }

  void post (Event *e) {
    Synchronized(this);
    EventQueue *eq = getQueue();
    e->post(eq);
  }

  void setEventQueue(Event *e) {
    Synchronized(this);
    Synchronized(e);
    e->setPreferredQueue(getQueue());
  }

  EventQueue *queue() {
    Synchronized(this);
    return q;
  }

  void setQueue(EventQueue *queue) {
    Synchronized(this);
    q = queue;
  }
};

}} //namespace

#endif // TEXTUS_EVENT_EVENT_FACTORY_H_
