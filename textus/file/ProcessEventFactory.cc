/* ProcessEventFactory.cc -*- c++ -*-
 * Copyright (c) 2010-2013 Ross Biro
 *
 * Events that represent what can go on with a process.
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

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/file/ProcessEventFactory.h"
#include "textus/file/FileHandleEventFactory.h"

namespace textus { namespace file {

using namespace textus::base;

class PEFFactory: public FileHandleEventFactory {
private:
  AutoWeakDeref<ProcessMonitor> parent;
  textus::base::logic::Bool stderr;
  textus::base::logic::Bool stdin;

public:
  explicit PEFFactory(ProcessMonitor *p): stderr(false) {
    parent = p;
  }

  virtual ~PEFFactory() {}

  virtual Event *errorEvent(Error *e);
  virtual Event *readEvent(string s);
  virtual Event *writeEvent();

  virtual Event *eofEvent();

  void setStderr(bool f) {
    stderr = f;
  }

  void setStdin(bool f) {
    stdin = f;
  }

};


Event *PEFFactory::writeEvent() {
  ProcessEvent *event;
  event = new ProcessEvent(parent);
  event->setCanWrite(true);
  setEventQueue(event);
  return event;
}

Event *PEFFactory::readEvent(string s) {
  ProcessEvent *event;
  event = new ProcessEvent(parent);
  event->setData(s);
  if (stderr) {
    event->setStderr(true);
  }
  setEventQueue(event);
  return event;
}

Event *PEFFactory::errorEvent(Error *e)
{
  ProcessEvent *event;
  event = new ProcessEvent(parent);
  event->setError(e);
  setEventQueue(event);
  return event;
}

Event *PEFFactory::eofEvent() {
  ProcessEvent *event;
  event = new ProcessEvent(parent);
  event->setEof(true);
  if (stderr) {
    event->setStderr(true);
  } else if (stdin) {
    event->setStdin(true);
  }
  setEventQueue(event);
  return event;
}


void ProcessEventFactory::setStdin(FileHandle *fh) {
  AUTODEREF(PEFFactory *, pef);
  pef = new PEFFactory(monitor());
  pef->setStdin(true);
  fh->setEventFactory(pef);
}

void ProcessEventFactory::setStdout(FileHandle *fh) {
  AUTODEREF(PEFFactory *, pef);
  pef = new PEFFactory(monitor());
  fh->setEventFactory(pef);
}

void ProcessEventFactory::setStderr(FileHandle *fh) {
  AUTODEREF(PEFFactory *, pef);
  pef = new PEFFactory(monitor());
  pef->setStderr(true);
  fh->setEventFactory(pef);
}

void ProcessEventFactory::monitorDeleted()
{
  setMonitor(NULL);
}    


}} //namespace
