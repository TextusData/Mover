/* SignalHandler.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
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
