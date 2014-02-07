/* CallBack.cc -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Holds information for doing a callback on one or more other threads.
 *
 */

#include "textus/base/ThreadCallBack.h"
#include "textus/base/Thread.h"

namespace textus { namespace base {

void ThreadCallBack::do_it() {
  CallBack::do_it();
  if (Thread::self()->getDoAll()) {
    Synchronized(this);
    doneCount_++;
  }
}

}} //namespace
