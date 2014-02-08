/* Shell.cc -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Execute a system command via the shell.
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
#include "textus/file/Shell.h"

#define SHELL_ENVIRONMENT_VARIABLE "shell"
#define DEFAULT_SHELL "/bin/sh"

namespace textus { namespace file {

void Shell::go() {

  Synchronized(this);
  if (shl.length() == 0) {
    const char *sh = getenv(SHELL_ENVIRONMENT_VARIABLE);
    if (sh == NULL) {
      sh = DEFAULT_SHELL;
    }
    shl = string(sh);
  }

  // Using acess is ok here since we are really
  // just trying to catch errors earlier.
  // If one of the races occurs, then we
  // will catch the error below rather than
  // here.
  if (access(shl.c_str(), X_OK) < 0) {
    AUTODEREF(Error *, e);
    e = new Error();
    AUTODEREF(ProcessEvent *, pe);
    pe = new ProcessEvent(this);
    pe->setError(e);
    pe->post();
    return;
  }

  const char *argv[5];
  argv[0] = shl.c_str();
  argv[1] = "-l";
  argv[2] = "-c";
  argv[3] = cmd.c_str();
  argv[4] = NULL;

  LOG(INFO) << "Executing shell command: " << cmd << "\n";

  createPipes();

  stdout()->reader()->watchZeroReads(true);
  stdin()->reader()->setNonBlocking(false);
  stdout()->writer()->setNonBlocking(false);
  stderr()->writer()->setNonBlocking(false);

  int sti=stdin()->reader()->nativeHandle();
  int sto=stdout()->writer()->nativeHandle();
  int ste=stderr()->writer()->nativeHandle();

  assert(sti > 2);
  assert(sto > 2);
  assert(ste > 2);

  GRAB_EXEC_BARRIER();
  pid_t pid = fork();
  if (!pid) {
    // Need to keep this short.  The more there is,
    // the more than can go wrong.
    //dup2(2, 3);
    if (dup2(sti, 0) < 0) {
      ::write(2, "duping stdin failed.\n", 22);
    }
    if (dup2(sto, 1) < 0) {
      ::write(2, "duping stdout failed.\n", 23);
    }
    if (dup2(ste, 2) < 0) {
      //::write(1, "duping  stderr failed.\n", 23);
    }
    ::close(sti);
    ::close(sto);
    ::close(ste);
    do {
      execv(shl.c_str(), const_cast<char * const *>(argv));
      usleep(100);
    } while (errno == EAGAIN);
    // We get here too often.  We need better error handling, and possibly
    // a loop to try more than once.
    char buff[256];
    snprintf(buff, sizeof(buff), "Unable to exec: %d\n", errno);
    //write(3, "Ooops, shouldn't be here.\n", 27);
    write(1, buff, strlen(buff));
    write(2, buff, strlen(buff));
    ::close(0);
    ::close(1);
    ::close(2);
    exit(1);
  }
  // parent process
  if (pid == -1) {
    AUTODEREF(Error *, e);
    e = new Error();
    AUTODEREF(ProcessEvent *, pe);
    pe = new ProcessEvent(this);
    pe->setError(e);
    pe->post();
    return;
  }
  setPid(pid);
  stdin()->reader()->close();
  stdout()->writer()->close();
  stderr()->writer()->close();
  if (stdout()->reader()->nativeHandle() == -1) {
    LOG(ERROR) << "Pipe has bad handle.\n";
  }
  addMonitor();
  if (in_buffer.length() > 0) {
    stdin()->writer()->closeOnEmptyBuffer();
    stdin()->writer()->addToOutBuffer(in_buffer);
  }
}

void Shell::read(ProcessEvent *pe) {
  Synchronized(this);
  buffer = buffer + pe->data();
  ProcessMonitor::read(pe);
}

void Shell::errRead(ProcessEvent *pe) {
  Synchronized(this);
  errBuffer = errBuffer + pe->data();
  ProcessMonitor::errRead(pe);
}


void Shell::sto_eof()  {
  Synchronized(this);
  sto_eof_seen = true;
  if (buffer ==  string("") ) {
    LOG(WARNING) << "no Output from cmd\n";
  }
  ProcessMonitor::sto_eof();
}

Shell::~Shell() {
};


}} //namespace
