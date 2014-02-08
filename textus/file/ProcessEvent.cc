/* ProcessEvent.cc -*- c++ -*-
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

#include "textus/file/ProcessEvent.h"
#include "textus/file/ProcessMonitor.h"

namespace textus { namespace file {

ProcessEvent::ProcessEvent(ProcessMonitor *p):FileEvent(static_cast<EventTarget *>(p)), 
					      did_finish(false), stderr(false), stdin(false), stat(-1)
{
}

ProcessMonitor *ProcessEvent::monitor()
{
  Synchronized(this);
  return dynamic_cast<ProcessMonitor*>(eventTarget());
}

void ProcessEvent::do_it() {
  Synchronized(this);
  ProcessMonitor *pm = monitor();
  if (pm) {
    if (error()) {
      pm->error(this);
    }
    if (data().length()) {
      if (stderr) {
	pm->errRead(this);
      } else {
	pm->read(this);
      }
    }
    if (canWrite()) {
      pm->canWrite(this);
    }
    if (did_finish) {
      pm->finished(this);
    }
    if (eof()) {
      if (stderr) {
	pm->ste_eof();
      } else if (stdin) {
	pm->sti_eof();
      } else {
	pm->sto_eof();
      }
    }
  }
}

}} //namespace
