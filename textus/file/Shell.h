/* Shell.h -*- c++ -*-
 * Copyright (c) 2010-2013 Ross Biro
 *
 * Execute a system command via the shell.
 */

#ifndef TEXTUS_FILE_SHELL_H_
#define TEXTUS_FILE_SHELL_H_

#include "textus/file/FileHandle.h"
#include "textus/file/ProcessMonitor.h"
#include "textus/file/Pipe.h"
#include "textus/file/ProcessEventFactory.h"
#include "textus/file/ProcessEvent.h"

namespace textus { namespace file {

class Shell: public ProcessMonitor {
private:
  string cmd;
  string shl;
  string buffer;
  string errBuffer;
  string in_buffer;
  bool sto_eof_seen;
  
public:
  Shell(): cmd(), shl(), buffer() , sto_eof_seen(false) {}
  virtual ~Shell();

  void go();

  virtual void read(ProcessEvent *);
  virtual void errRead(ProcessEvent *e);

  string shell() const {
    return shl;
  }

  void setShell(string s) {
    shl = s;
  }

  void setInput(string d) {
    in_buffer = d;
  }

  string command() const {
    return cmd;
  }

  void setCommand(string c) {
    cmd = c;
  }
  
  string getOutput() {
    Synchronized(this);
    assert(sto_eof_seen);
    //assert(buffer != string(""));
    return buffer;
  }

  string getErrorOutput() {
    Synchronized(this);
    return errBuffer;
  }

  virtual int exitStatus() {
    LOG(DEBUG) << "Shell otuput: " << getOutput() << "\n";
    LOG(DEBUG) << "Shell error: " << getErrorOutput() << "\n";
    return ProcessMonitor::exitStatus();
  }

  virtual void sto_eof();
};

}} // namespace

#endif // TEXTUS_FILE_SHELL_H_
