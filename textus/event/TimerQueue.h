/* TimerQueue.h -*- c++ -*-
 * Copyright (c) 2009, 2013, 2014 Ross Biro
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

/*
 * Lock order is timer then queue.
 */

#ifndef TEXTUS_EVENT_TIMERQUEUE_H
#define TEXTUS_EVENT_TIMERQUEUE_H

#include "textus/event/Timer.h"
#include "textus/base/Thread.h"
#include "textus/base/ReferenceList.h"

namespace textus { namespace event {

using namespace textus::base::logic;
using namespace textus::base;

class TimerQueue: virtual public Base {
private:
  // Lock order, Timer, then list.
  ReferenceList<Timer *> timer_list;
  Condition condition;
  Mutex mutex;
  Bool wake_up;
  Bool should_exit;
  class Thread *timerQueueThread;

  typedef ReferenceList<Timer *>::iterator tl_iterator;

protected:
  int weakDelete() {
    Synchronized(this);
    for (tl_iterator i = timer_list.begin();
	 i != timer_list.end(); ++i) {
      (*i)->timerStop();
    }
    timer_list.clear();
    return Base::weakDelete();
  }

public:
  TimerQueue():timer_list(), condition(), mutex(), wake_up(false), should_exit(false), timerQueueThread(NULL)
  {
  }

  virtual ~TimerQueue();

  static TimerQueue *defaultTimerQueue();

  void addTimer(Timer *t)
  {
    Synchronized(this);
    if (timer_list.empty()) {
      timer_list.push_front(t);
      newAlarmTime();
      return;
    }

    tl_iterator i = timer_list.begin();
    if (t->endTime().before((*i)->endTime())) {
      timer_list.push_front(t);
      newAlarmTime();
      return;
    }

    for (; i != timer_list.end(); ++i) {
      if (t->endTime().before((*i)->endTime())) {
	timer_list.insert(i, t);
	return;
      }
    }
    timer_list.push_back(t);
  }

  void removeTimer(Timer *t)
  {
    Synchronized(this);
    timer_list.erase(t);
    newAlarmTime();
    return;
  }

  void wake()
  {
    LOCK(&mutex);
    wake_up = true;
    condition.signal();
  }

  void newAlarmTime()
  {
    wake();
  }

  void allowSleep()
  {
    LOCK(&mutex);
    wake_up = false;
  }

  void waitForTimeOut()
  {
    Time timeout;
    {
      Synchronized(this);
      allowSleep();
      if (timer_list.empty()) {
	timeout = Time::maxTime();
      } else {
	Timer *t = timer_list.front();
	timeout = t->endTime();
      }

      // Have to grab the mutex before
      // droping the lock on this so that
      // we don't have someone change the
      // start time with out us noticing.
      mutex.lock();
    }
    if (wake_up || should_exit) {
      mutex.unlock();
      return;
    }
    condition.waitTimeOut(mutex, timeout);
    mutex.unlock();
  }

  void processTimeOuts() {
    ReferenceList<Timer *> expired;
    {
      // have to drop the lock before
      // processing.  Otherwise we have
      // a lock order problem.
      Synchronized(this);
      Time now = Time::now();
      while (!timer_list.empty()) {
	tl_iterator i = timer_list.begin(); 
	if (now.after((*i)->endTime())) {
	  expired.push_back(*i);
	  timer_list.erase(i);
	} else {
	  break;
	}
      }
    }
    foreach (i, expired) {
      (*i)->timeOut();
    }
    expired.clear();
  }

  // Does not return until wake is called.
  void timerThreadLoop()
  {
    LOCK(&mutex);
    while (!should_exit) {
      mutex.unlock();
      waitForTimeOut();
      processTimeOuts();
      mutex.lock();
    }
  }

  bool shouldExit()
  {
    LOCK(&mutex);
    return should_exit;
  }

  void setShouldExit(bool t)
  {
    {
      LOCK(&mutex);
      should_exit = t;
    }
    if (t) {
      wake();
    }
  }

  void setThread(Thread *t)
  {
    Synchronized(this);
    if (timerQueueThread) {
      timerQueueThread->deref();
    }
    timerQueueThread = t;
    if (timerQueueThread) {
      timerQueueThread->ref();
    }
  }

  Thread *thread()
  {
    Synchronized(this);
    return timerQueueThread;
  }

  void threadExit()
  {
    {
      Synchronized(this);
      setShouldExit(true);
    }
    timerQueueThread->join();
  }
};
}} //namespace
#endif // TEXTUS_EVENT_TIMERQUEUE_H
