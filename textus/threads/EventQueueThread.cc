/* EventQueueThread.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Handle eventQueue threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one eventQueue thread per cpu is likely ideal.
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
