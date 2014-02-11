/* Event.cc -*- c++ -*-
 * Copyright (c) 2009, 2013, 2014 Ross Biro
 *
 * This class is the core of the event dispatching loop.
 * We use this to transfer signals back and forth, let other
 * threads no something has happened, etc.
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

#include "textus/event/Event.h"
#include "textus/event/EventQueue.h"

namespace textus { namespace event {

Event::~Event() 
{
}

void Event::setPreferredQueue(EventQueue *eq)
{
  Synchronized(this);
  preferred_queue = eq;
}

void Event::timeOut() 
{
  Synchronized(this);
  if (current_queue) {
    LOG(WARNING) << "Event " << this <<" Timed out on Queue " << current_queue;
    current_queue->timeOut(this);
  }
}

void Event::post(EventQueue *queue) 
{
  if (textus::base::init::shutdown_started) {
    return;
  }
  Synchronized(this);
  if (!queue) {
    queue = EventQueue::defaultQueue();
  }

  if (current_queue) {
    LOG(WARNING) << "Event switching from queue " << current_queue << " to queue " << queue << "."
		 << "Originally posted " << posted.elapsedTime() << ".  System Likely unstable.\n";
  }

  if (!queue) {
    LOG(ERROR) << "Event Posted with NULL Queue.\n";
    LOG_CALL_STACK(ERROR);
    die();
  }

  posted = Time::now();
  current_queue = queue;
  this->timerStart(defaultTimeOut());
  queue->post(this);
}

void Event::post()
{
  Synchronized(this);
  post(preferred_queue);
}

void Event::repost()
{
  Synchronized(this);
  if (reposted) {
    LOG(ERROR) << "Event reposted twice.  System Assumed to be Unstable\n";
    die();
  }

  reposted = true;
  if (current_queue == EventQueue::failSafeQueue()) {
    LOG(ERROR) << "Event reposted from default queue.  System assumed to be unstable.\n";
    die();
  }

  post(EventQueue::failSafeQueue());
}

void Event::handle()
{
  Synchronized(this);
  if (preferred_queue) {
    preferred_queue = NULL;
  }
     
  if (current_queue) {
    current_queue->recordTimeOnQueue(posted.elapsedTime());
    current_queue = NULL;
  }

  if (target==NULL || target->eventReceived(this) == 0) {
    do_it();
  }

  if (target) {
    target->eventDone(this);
  }
}

Duration Event::defaultTimeOut()
{
  return Duration::seconds(10000000);
}

void Event::close() {
  Synchronized(this);
  timerStop();
  if(current_queue) {
    current_queue->remove(this);
  }
  current_queue = NULL;
}

}} // namespace
