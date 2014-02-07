/* ConditionTimer.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * This timer event subclass signals a condition when it goes off.
 */

#ifndef TEXTUS_EVENT_CONDTION_TIMER_EVENT_H_
#define TEXTUS_EVENT_CONDTION_TIMER_EVENT_H_

#include "textus/event/Timer.h"
#include "textus/base/locks/Condition.h"
#include "textus/base/locks/Mutex.h"
#include "textus/base/logic/Bool.h"
#include "textus/event/TimerEvent.h"

namespace textus { namespace event {

using namespace textus::base::logic;
using namespace textus::base::locks;

class ConditionTimerEvent: public TimerEvent {
private:
  Condition cond;
  Mutex mutex;
  Bool set;

protected:
  void do_it();

public:
  ConditionTimerEvent():cond(), mutex(), set(false) {}
  ~ConditionTimerEvent() {}

  int wait();
  void reset() {
    LOCK(&mutex);
    set = false;
  }
		   
};

}} //namespace

#endif //TEXTUS_EVENT_CONDITION_TIMER_H_
