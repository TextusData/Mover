/* ConditionTimer.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * This timer subclass signals a condition when it goes off.
 */

#include "textus/event/ConditionTimer.h"

namespace textus { namespace event {
void ConditionTimer::timeOut()
{
  LOCK(&mutex);
  set = true;
  cond.signal();
}

int ConditionTimer::wait()
{
  LOCK(&mutex);
  if (set)
    return 0;
  cond.wait(mutex);
  if (set)
    return 0;
  return 1;
}

}} //namespace
