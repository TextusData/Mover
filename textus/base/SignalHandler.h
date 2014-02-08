/* SignalHandler.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Encapsulate signal handling so
 * that we can easily catch signals
 * such as SIGUSR1 and SIGILL and
 * act on them.  
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

#ifndef TEXTUS_BASE_SIGNALHANDLER_H_
#define TEXTUS_BASE_SIGNALHANDLER_H_

#include <signal.h>

#include "textus/base/Base.h"
#include "textus/base/ThreadLocal.h"
#include "textus/base/logic/Bool.h"

namespace textus { namespace base {

class SignalHandler: virtual public Base {
public:
  enum actions { HANDLE, IGNORE };
  
private:
  static ThreadLocal<SignalHandler **> handlers;
  static void handler(int signum, siginfo_t *si, void *context);
  textus::base::logic::Bool installed;
  int signum;
  int sigs_caught;
  actions act;
  struct sigaction oldact;
  SignalHandler *oldhandler;

protected:
  virtual void do_it() {}

  int reinstall() {
    int ret = 0;
    Synchronized(this);
    struct sigaction newact;
    memset(&newact, 0, sizeof(newact));
    newact.sa_sigaction = handler;
    newact.sa_flags = SA_RESTART | SA_SIGINFO;
    HRC(sigaction(signum, &newact, &oldact));
    installed = true;
    if (handlers == NULL) {
      handlers = static_cast<SignalHandler **>(malloc (sizeof(SignalHandler *) * _NSIG));
    }
    handlers[signum] = this;
  error_out:
    return ret;
  }

  int install() {
    int ret=0;
    Synchronized(this);
    if (handlers) {
      oldhandler = handlers[signum];
    } else {
      oldhandler = NULL;
    }
    HRC(reinstall());

  error_out:
    return ret;
  }

  int uninstall() {
    int ret = 0;
    Synchronized(this);
    if (handlers == NULL || handlers[signum] != this || !installed) {
      LOG(ERROR) << "uninstalling signal handler that isn't installed.\n";
      die();
    }
    installed = false;
    handlers[signum] = oldhandler;
    if (oldhandler) {
      oldhandler->reinstall();
    } else {
      HRC(sigaction(signum, &oldact, NULL));
    }
    return ret;

  error_out:
    LOG(ERROR) << "uninstalling signal handler failed.\n";
    die();
    return -1;
  }

public:
  explicit SignalHandler():installed(false), signum(0), sigs_caught(0),
			oldhandler(NULL) {}
  virtual ~SignalHandler() {
    cleanup();
  }

  void cleanup() {
    Synchronized(this);
    if (installed) {
      uninstall();
      installed = false;
    }
  }

  void setsignal(int sig) {
    Synchronized(this);
    signum = sig;
  }

  int setaction(actions a) {
    Synchronized(this);
    act = a;
    return install();
  }

  int signals_caught() const {
    return sigs_caught;
  }

};

}} //namespace

#endif //TEXTUS_BASE_SIGNALHANDLER_H_
