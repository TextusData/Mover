/* Event.h -*- c++ -*-
 * Copyright (c) 2009-2010, 2013 Ross Biro
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
 * Lock order is Event, then EventQueue.
 */

#ifndef TEXTUS_EVENT_EVENT_H
#define TEXTUS_EVENT_EVENT_H

#include "textus/event/Timer.h"
#include "textus/base/Base.h"
#include "textus/base/logic/Bool.h"
#include "textus/base/Time.h"
#include "textus/event/EventWatcher.h"

namespace textus { namespace event {
using namespace textus::base::logic;


class EventQueue;

class Event: public Timer {
private:
  AutoDeref<EventTarget> target;
  AutoDeref<EventQueue> preferred_queue;
  AutoDeref<EventQueue> current_queue;
  Bool reposted;
  Time posted;

  friend class EventQueue;

  static Duration defaultTimeOut();

protected:
  virtual void do_it() = 0;

  virtual void timeOut();

public:
  Event(EventTarget *t): target(t), reposted(false)
  {
  }

  virtual ~Event();

  void post(EventQueue *queue);
    
  void post();

  void repost();

  void handle();

  Bool wasReposted() {
    return reposted;
  }

  void setPreferredQueue(EventQueue *eq);

  EventQueue *preferredQueue() {
    Synchronized(this);
    return preferred_queue;
  };

  void setEventTarget(EventTarget *et) {
    Synchronized(this);
    target = et;
  }

  EventTarget *eventTarget() {
    Synchronized(this);
    return target;
  }

  void close(); // Remove the event from the queue and deactivate it.

};

}} //namespace

#endif //TEXTUS_EVENT_EVENT_H
