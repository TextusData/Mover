/* ConditionTimer.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * This timer subclass signals a condition when it goes off.
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

#ifndef TEXTUS_EVENT_CONDITION_TIMER_H_
#define TEXTUS_EVENT_CONDITION_TIMER_H_

#include "textus/event/Timer.h"
#include "textus/base/locks/Condition.h"
#include "textus/base/locks/Mutex.h"
#include "textus/base/logic/Bool.h"

namespace textus { namespace event {
using namespace textus::base::logic;
using namespace textus::base::locks;

class ConditionTimer: public Timer {
private:
  Condition cond;
  Mutex mutex;
  Bool set;

protected:
  void timeOut();

public:
  ConditionTimer():cond(), mutex(), set(false) {}
  ~ConditionTimer() {}

  int wait();
  void reset() {
    LOCK(&mutex);
    set = false;
  }
		   
};

}} //namespace

#endif //TEXTUS_EVENT_CONDITION_TIMER_H_
