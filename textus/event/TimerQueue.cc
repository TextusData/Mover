/* TimerQueue.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * This class is the core of the event dispatching loop.
 * We use this to transfer signals back and forth, let other
 * threads no something has happened, etc.
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

#include "textus/event/TimerQueue.h"
#include "textus/base/init/Init.h"
#include "textus/base/Thread.h"

namespace textus { namespace event {

static Base *timerQueueThread(Base *b)
{
  TimerQueue *tq = dynamic_cast<TimerQueue *>(b);
  if (!tq) {
    LOG(ERROR) << "timerQueueThread unable to convert base (" << b << ") to TimerQueue * \n";
    tq = TimerQueue::defaultTimerQueue();
    LOG(ERROR) << "Using default TimerQueue (" << tq << ").\n";
  }
  tq->timerThreadLoop();
  return NULL;
}

DEFINE_INIT_FUNCTION(startTimerQueueThread, AUXILLARY_THREAD_INIT_PRIORITY)
{
  TimerQueue *tq = TimerQueue::defaultTimerQueue();
  Thread *th = new Thread(timerQueueThread, static_cast<Base *>(tq));
  tq->setThread(th);
  return 0;
}

DEFINE_SHUTDOWN_FUNCTION(endTimerQueueThread, AUXILLARY_THREAD_SHUTDOWN_PRIORITY)
{
  TimerQueue *tq = TimerQueue::defaultTimerQueue();
  tq->threadExit();
}

TimerQueue::~TimerQueue()
{
  timerQueueThread->deref();
}

TimerQueue *TimerQueue::defaultTimerQueue()
{
  return Timer::defaultTimerQueue();
}

}} //namespace
