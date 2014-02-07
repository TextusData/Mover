/* Timer.cc -*- c++ -*-
 * Copyright (c) 2009, 2013 Ross Biro
 *
 * This class is the core of the event dispatching loop.
 * We use this to transfer signals back and forth, let other
 * threads know something has happened, etc.
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

  Synchronized(this);
  start_time = Time::now();
  end_time = start_time + time_out;
  queue->addTimer(this);
  tq = queue;
}

void Timer::timerStop() {
  Synchronized(this);
  tq->removeTimer(this);
  tq = NULL;
}

TimerQueue *Timer::defaultTimerQueue()
{
  static TimerQueue static_default_timer_queue;
  return &static_default_timer_queue;
}

}} //namespace
