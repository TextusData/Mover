/* TimerEvent.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * An event that posts itself after a predetermined about of time.
 * Similiar to Timer, but Timer doesn't use events since the
 * EventQueue timeouts are built on it.
 */

#include "textus/event/TimerEvent.h"

namespace textus { namespace event {

void TimerEvent::timeOut()
{
  if (!first_time_out) {
    Event::timeOut();
    return;
  }
  first_time_out = false;
  post();
}

}} // namespace
