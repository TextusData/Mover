/* ProcessEvent.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Events that represent what can go on with a process.
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
