/* ProcessEventFactory.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
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

#ifndef TEXTUS_FILE_PROCESS_EVENT_FACTORY_H_
#define TEXTUS_FILE_PROCESS_EVENT_FACTORY_H_

#include "textus/base/Base.h"
#include "textus/file/ProcessEvent.h"
#include "textus/file/FileHandle.h"
#include "textus/event/EventFactory.h"

namespace textus { namespace file {

class ProcessEventFactory: public EventFactory {
private:
  AutoWeakDeref<ProcessMonitor> pm;

public:
  explicit ProcessEventFactory(ProcessMonitor *p) {
    pm = p;
  }
  virtual ~ProcessEventFactory() {}

  virtual ProcessEvent *complete(int s) {
    ProcessEvent *pe = new ProcessEvent(pm);
    pe->setStatus(s);
    pe->setFinished(true);
    setEventQueue(pe);
    return pe;
  }

  void monitorDeleted();

  void setStdin(FileHandle *fh);
  void setStdout(FileHandle *fh);
  void setStderr(FileHandle *fh);

  ProcessMonitor *monitor() {
    return pm;
  }

  void setMonitor(ProcessMonitor *p) {
    pm = p;
  }
  
};

}} //namespace

#endif // TEXTUS_FILE_PROCESS_EVENT_FACTORY_H_
