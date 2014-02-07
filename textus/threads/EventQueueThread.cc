/* EventQueueThread.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Handle eventQueue threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one eventQueue thread per cpu is likely ideal.
 */

#include "textus/threads/EventQueueThread.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace threads {

using namespace textus::base;

Base *EventQueueThread::threadStartup(Base *b) {
  textus::event::EventQueue *eq = dynamic_cast<textus::event::EventQueue *>(b);
  textus::event::EventQueue::setPreferredQueue(eq);
  eq->threadLoop();
  textus::event::EventQueue::setPreferredQueue(NULL);
  return NULL;
}

}} //namespace
