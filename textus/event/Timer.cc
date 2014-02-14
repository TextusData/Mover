/* Timer.cc -*- c++ -*-
 * Copyright (c) 2009, 2013, 2014 Ross Biro
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
#include "textus/event/Timer.h"
#include "textus/event/TimerQueue.h"

namespace textus { namespace event {

Timer::Timer(): start_time(), end_time()
{
}

Timer::~Timer()
{
}

void Timer::timerStart(Duration time_out, TimerQueue *queue)
{
  if (queue == NULL) {
    queue = defaultTimerQueue();
  }

  {
    // can't hold this lock while
    // we put the timer on the queue.
    // The lock order is queue, then
    // timer.  We would have it backwards.
    Synchronized(this);
    start_time = Time::now();
    end_time = start_time + time_out;
    tq = queue;
  }
  queue->addTimer(this);

}

void Timer::timerStop() {
  Synchronized(this);
  if (tq == NULL ){
    return;
  }
  tq->removeTimer(this);
  tq = NULL;
}

TimerQueue *Timer::defaultTimerQueue()
{
  static TimerQueue static_default_timer_queue;
  return &static_default_timer_queue;
}

}} //namespace
