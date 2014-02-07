/* ConditionTimerEvent.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * This timer subclass signals a condition when it goes off.
 */

#include "textus/event/ConditionTimerEvent.h"

namespace textus { namespace event {
void ConditionTimerEvent::do_it()
{
  LOCK(&mutex);
  set = true;
  cond.signal();
}

int ConditionTimerEvent::wait()
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
