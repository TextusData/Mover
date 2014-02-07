/* SignalHandler.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 */

#include "textus/base/SignalHandler.h"

namespace textus { namespace base {
ThreadLocal<SignalHandler **> SignalHandler::handlers;

void SignalHandler::handler(int signum, siginfo_t *si, void *context) {
  /* XXXXXX FIXME:  this is likely not signal safe. */
  if (handlers == NULL) {
    return;
  }
  SignalHandler *sh = handlers[signum];
  sh->sigs_caught++;
  switch (sh->act) {
  case IGNORE:
    break;

  case HANDLE:
    sh->do_it();
    break;
  }
  return;
}

}} //namespace
