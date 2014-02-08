/* EventWatcher.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * A class that watches events.
 *
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

#ifndef TEXTUS_EVENT_EVENT_WATCHER_H_
#define TEXTUS_EVENT_EVENT_WATCHER_H_

#include "textus/base/Base.h"
#include "textus/base/ReferenceList.h"

namespace textus { namespace event {
using namespace textus::base;
class Event;

class EventWatcher: virtual public Base {
private:

protected:
  virtual int weakDelete() {
    return Base::weakDelete();
  }

public:
  EventWatcher() {}
  virtual ~EventWatcher() {}

  virtual int eventReceived(Event *e) { return 0; }
  virtual int eventDone(Event *e) { return 0;}
  virtual void signalReceived() { return; }

};


class EventTarget: virtual public Base {
private:
  ReferenceList<EventWatcher *> watchers;

public:
  EventTarget() {}
  virtual ~EventTarget() {}

  virtual void signal() {
    Base::signal();
    Synchronized(this);
    if (watchers.empty()) {
      return;
    }

    ReferenceList<EventWatcher *>::iterator n;
    for (ReferenceList<EventWatcher *>::iterator i = watchers.begin(); i != watchers.end(); i = n) {
      n = i;
      ++n;
      (*i)->signalReceived();
    }
    return;
  }

  void addWatcher(EventWatcher *ew) {
    Synchronized(this);
    watchers.push_back(ew);
  }

  void removeWatcher(EventWatcher *ew) {
    Synchronized(this);
    watchers.erase(ew);
  }

  int eventReceived(Event *e) {
    Synchronized(this);
    if (watchers.empty()) {
      return 0;
    }
    for (ReferenceList<EventWatcher *>::iterator i = watchers.begin(); i != watchers.end(); ++i) {
      int ret = (*i)->eventReceived(e);
      if (ret != 0) {
	return ret;
      }
    }
    return 0;
  }

  int eventDone(Event *e) {
    Synchronized(this);
    if (watchers.empty()) {
      return 0;
    }
    for (ReferenceList<EventWatcher *>::iterator i = watchers.begin(); i != watchers.end(); ++i) {
      int ret = (*i)->eventDone(e);
      if (ret != 0) {
	return ret;
      }
    }
    return 0;
  }


  
};

}} //namespace

#endif // TEXTUS_EVENT_EVENT_WATCHER_H_
