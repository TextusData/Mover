/* TimerEvent.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * An event that posts itself after a predetermined about of time.
 * Similiar to Timer, but Timer doesn't use events since the
 * EventQueue timeouts are built on it.
 */

#ifndef TEXTUS_EVENT_TIMER_EVENT_H_
#define TEXTUS_EVENT_TIMER_EVENT_H_

#include "textus/event/Event.h"
#include "textus/event/EventQueue.h"

namespace textus { namespace event {

class TimerEvent: public Event {
private:
  Bool first_time_out;

protected:
  // Overload this so we can use the builtin timer.
  virtual void timeOut();

public:
  TimerEvent():Event(NULL), first_time_out(true) {}

  explicit TimerEvent(Duration d):Event(NULL), first_time_out(true) 
  {
    timerStart(d);
  }

  virtual ~TimerEvent() {}

  void timerStart(Duration time_out, TimerQueue *queue = NULL)
  {
    // Have to do this here.  The event will be posted in a different
    // thread.
    setPreferredQueue(EventQueue::preferredQueue());
    Event::timerStart(time_out, queue);
  }

  void timerStop() {
    Event::timerStop();
    close();
  }

};

}} //namespace

#endif // TEXTUS_EVENT_TIMER_EVENT_H_
