/* Timer.h -*- c++ -*-
 * Copyright (c) 2009, 2013 Ross Biro
 *
 * This class is the core of the event dispatching loop.
 * We use this to transfer signals back and forth, let other
 * threads know something has happened, etc.
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
 * Lock order is timer then queue.
 */

#ifndef TEXTUS_EVENT_TIMER_H
#define TEXTUS_EVENT_TIMER_H

#include "textus/base/Base.h"
#include "textus/base/Time.h"

namespace textus { namespace event {
using namespace textus::base;

class TimerQueue;

class Timer: virtual public Base {
private:
  friend class TimerQueue;
  Time start_time;
  Time end_time;
  AutoWeakDeref<TimerQueue> tq;

protected:
  virtual void timeOut() = 0;

public:

  static TimerQueue *defaultTimerQueue();
  explicit Timer();
  virtual ~Timer();
    
  void timerStart(Duration time_out, TimerQueue *queue = NULL);
  void timerStop();

  const Duration elapsedTime()
  {
    return Time::now() - start_time;
  }

  const Time &endTime() 
  {
    return end_time;
  }
};
}} // namespace

#endif //TEXTUS_EVENT_TIMER_H
