/* ProcessEvent.h -*- c++ -*-
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

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/base/AutoDeref.h"
#include "textus/base/Error.h"
#include "textus/file/ProcessMonitor.h"
#include "textus/file/FileEvent.h"
#include "textus/event/EventWatcher.h"
#include <string>

#ifndef TEXTUS_FILE_PROCESS_EVENT_H_
#define TEXTUS_FILE_PROCESS_EVENT_H_

namespace textus { namespace file {

using namespace textus::base;

class ProcessMonitor;

class ProcessEvent: public FileEvent {
private:
  bool did_finish;
  bool stderr; // buffer came from stderr.
  bool stdin;  // event came from stdin.
  int stat;    // process exit status.

protected:
  virtual void do_it();

public:
  explicit ProcessEvent(ProcessMonitor *p);

  virtual ~ProcessEvent() {}

  bool finished() {
    Synchronized(this);
    return did_finish;
  }

  void setFinished(bool f) {
    Synchronized(this);
    did_finish = f;
  }
  
  void setStderr(bool f) {
    Synchronized(this);
    stderr = f;
  }

  void setStdin(bool f) {
    Synchronized(this);
    stdin = f;
  }

  int status() {
    Synchronized(this);
    return stat;
  }

  void setStatus(int s) {
    Synchronized(this);
    stat = s;
  }

  ProcessMonitor *monitor();

};

}} // namespace

#endif //TEXTUS_FILE_PROCESS_EVENT_H_
