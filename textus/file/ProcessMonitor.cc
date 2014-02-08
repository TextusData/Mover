/* ProcessMonitor.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Keep an eye on a process
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

#include "textus/file/ProcessMonitor.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/base/locks/Completion.h"
#include "textus/file/ProcessEventFactory.h"
#include "textus/base/Thread.h"

#include <sys/wait.h>
#include <signal.h>

#include <map>

namespace textus { namespace file {

static Completion reaper_started;

static ReferenceValueMap<pid_t, ProcessMonitor *> pid_map;
static map<pid_t, int> unknown_pids;
static Bool reaper_thread_done=false;
static Thread *reaper_thread;

void ProcessMonitor::errRead(ProcessEvent *e) {
}

ProcessMonitor::ProcessMonitor(): done(false), readDone(false), errDone(false), inDone(false)
{
  AUTODEREF(ProcessEventFactory *, p);
  p = new ProcessEventFactory(this);
  pef = p;
}

void ProcessMonitor::wakeReaper() {
  if (reaper_thread) {
    reaper_thread->signal();
  }
}

void ProcessMonitor::createPipes() {
  AUTODEREF(Pipe *, sti);
  AUTODEREF(Pipe *, sto);
  AUTODEREF(Pipe *, ste);
 
  sti = new Pipe();
  stdin_pipe = sti;

  sto = new Pipe();
  stdout_pipe = sto;

  ste = new Pipe();
  stderr_pipe = ste;

  pef->setStdin(sti->writer());
  pef->setStdout(sto->reader());
  pef->setStderr(ste->reader());
}

void ProcessMonitor::addMonitor() {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&pid_map);
  reaper_started.wait();
  pid_t p = pid();
  {
    // check to see if the thread completed before the process monitor was ready.
    Synchronized(reaper_thread);
    if (unknown_pids.find(p) != unknown_pids.end()) {
      // Already Done.
      AUTODEREF(ProcessEvent *, pe);
      pe = eventFactory()->complete(unknown_pids[p]);
      pe->post();
      unknown_pids.erase(p);
      return;
    }
  }
  Synchronized(this);
  pid_map[p] = this;
  wakeReaper();
}

int ProcessMonitor::weakDelete()
{
  pef->monitorDeleted();
  pef = NULL;

  if (stdin_pipe != NULL) {
    stdin_pipe->writer()->setEventFactory(NULL);
  }

  if (stdout_pipe != NULL) {
    stdout_pipe->reader()->setEventFactory(NULL);
  }

  if (stderr_pipe != NULL) {
    stderr_pipe->reader()->setEventFactory(NULL);
  }

  stdin_pipe = NULL;
  stdout_pipe = NULL;
  stderr_pipe = NULL;
  return Base::weakDelete();
}

void sigChildHandler(int sig) {
  static int sig_child_count=0;
  sig_child_count++;
}


DEFINE_INIT_FUNCTION(shellInit, THREAD_INIT_PRIORITY - 1) {
  //signal(SIGCHLD, SIG_IGN);
  return 0;
}

Base *reaperThreadStartup(Base *b) {
  sigset_t set;
  sigset_t oset;
  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  pthread_sigmask(SIG_UNBLOCK, &set, &oset);
  reaper_thread = Thread::self();
  reaper_thread->ref();
  reaper_thread->setDoAll(false);
  reaper_started.complete();
  while (!reaper_thread_done) {
    int stat_loc;
    pid_t pid;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, &oset);
    // Race condition.  If signal comes through
    // before wait and after the check, we will
    // block.
    while (Thread::self()->waitingTasks(NULL));
    pid = ::wait(&stat_loc);
    if (pid <= 0 && errno == ECHILD) {
      sigset_t sigmask;
      sigemptyset(&sigmask);
      sigsuspend(&sigmask);
      pthread_sigmask(SIG_SETMASK, &oset, &set);
      continue;
    }
    pthread_sigmask(SIG_SETMASK, &oset, &set);
    if (pid <= 0) {
      AUTODEREF(Error *,e);
      e = new Error();
      LOG(ERROR) << "reaper thread: " << e << "\n";
      continue;
    }
    assert(!textus::base::init::shutdown_done);
    Synchronized(&pid_map);
    if (pid_map.find(pid) == pid_map.end()) {
      LOG(WARNING) << "Process monitor waited for pid " << pid << " with no monitor. Possible race condition.\n";
      Synchronized(reaper_thread);
      unknown_pids[pid] = stat_loc;
      continue;
    }
    AutoDeref<ProcessMonitor> pm;
    ProcessMonitor *x;
    x = pid_map[pid];
    pm = x;
    pid_map.erase(pid);
    AUTODEREF(ProcessEvent *, pe);
    pe = pm->eventFactory()->complete(stat_loc);
    pe->post();
    continue;
  }
  return NULL;
}

static void shutdownReaper() 
{
  reaper_thread_done = true;

  ProcessMonitor::wakeReaper();
  if (reaper_thread) {
    reaper_thread->join();
    Synchronized(reaper_thread);
    unknown_pids.clear();
    reaper_thread->deref();
    reaper_thread = NULL;
  }
}

INITTHREAD(reaperThreadStartup, NULL);

SHUTDOWNTHREAD(shutdownReaper);

}} //namespace
