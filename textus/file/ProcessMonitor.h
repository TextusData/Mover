/* ProcessMonitor.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Keep an eye on a process
 */

#ifndef TEXTUS_FILE_PROCESS_MONITOR_H_
#define TEXTUS_FILE_PROCESS_MONITOR_H_

#include "textus/base/Base.h"
#include "textus/base/logic/Bool.h"
#include "textus/file/ProcessEventFactory.h"
#include "textus/file/ProcessEvent.h"
#include "textus/file/Pipe.h"
#include "textus/event/EventWatcher.h"

namespace textus { namespace file {

class ProcessEventFactory;

class ProcessMonitor: public textus::event::EventTarget {
private:
  bool done;
  bool readDone;
  bool errDone;
  bool inDone;
  pid_t pid_;
  AutoDeref<ProcessEventFactory> pef;
  int status;
  AutoDeref<Pipe> stdin_pipe;
  AutoDeref<Pipe> stdout_pipe;
  AutoDeref<Pipe> stderr_pipe;

protected:
  virtual int weakDelete();

public:
  ProcessMonitor();

  virtual ~ProcessMonitor() {}

  static void wakeReaper();

  void addMonitor();

  virtual int exitStatus() {
    if (!WIFEXITED (status)) return -1;
    return WEXITSTATUS(status);
  }

  virtual void error(ProcessEvent *e) {
    LOG(ERROR) << "Process Monitor: " << e->error() << "\n";
  }

  virtual void finished(ProcessEvent *e) {
    complete(e->status());
  }

  virtual void canWrite(ProcessEvent *e) {}

  virtual void read(ProcessEvent *e) {}

  virtual void errRead(ProcessEvent *e);

  virtual void sto_eof() {
    Synchronized(this);
    readDone = true;
    signal();
  }

  virtual void ste_eof() {
    Synchronized(this);
    errDone = true;
    signal();
  }

  virtual void sti_eof() {
    Synchronized(this);
    inDone = true;
    signal();
  }

  void waitForCompletion() {
    Synchronized(this);
    while (!done || !readDone || !errDone) {
      wait();
    }
  }

  void complete(int s) {
    Synchronized(this);
    status = s;
    done = true;
    signal();
  }

  pid_t pid() const {
    return pid_;
  }

  void setPid(pid_t p) {
    pid_ = p;
  }

  ProcessEventFactory *eventFactory() {
    return pef;
  }

  void setEventFactory(ProcessEventFactory *p) {
    pef = p;
  }

  void createPipes();

  Pipe *stdin() {
    return stdin_pipe;
  }

  Pipe *stdout() {
    return stdout_pipe;
  }

  Pipe *stderr() {
    return stderr_pipe;
  }

};

}} //namespace

#endif // TEXTUS_FILE_PROCESS_MONITOR_H_
