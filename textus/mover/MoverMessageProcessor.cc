/* MoverMessageProcssor.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
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

#include "textus/mover/MoverMessageProcessor.h"

namespace textus { namespace mover {

MMPTimerEvent::MMPTimerEvent(Duration d, MoverMessageProcessor *mmpin): TimerEvent(d), mmp_(mmpin)  {
}

MMPTimerEvent::~MMPTimerEvent() {
  setmmp(NULL);
}

void MMPTimerEvent::timeOut() {
  AUTODEREF(MoverMessageProcessor *, m);
  m = NULL;
  {
    Synchronized(this);
    if (mmp() != NULL) {
      m = mmp();
      m->ref();
    }
    close();
  }
  if (m != NULL) {
    m->timeOut();
  }
  textus::event::TimerEvent::timeOut();
}

void MMPTimerEvent::do_it() {
}

void MMPTimerEvent::close() {
  Synchronized(this);
  setmmp(NULL);
  TimerEvent::close();
}

MoverMessageProcessor::MoverMessageProcessor(SecureMessageServer<MoverMessageProcessor> *p):parent(p),
											    hello_count(0),
											    waiting_for_messages(true),
											    sending_messages(true),
											    sent_i_have(false),
											    need_startup_(false)
{
  last_message = Time::now();
  first_message = Time::now();
  connection_timeout=Random::rand(mover_connection_timeout);
  max_connection_time=Random::rand(mover_max_connection_time - mover_min_connection_time) + mover_min_connection_time;
  AUTODEREF(MMPTimerEvent *, te);
  te = new MMPTimerEvent(Duration::seconds(Random::rand(mover_random_message_time)+600), this);
  timer = te;
  if (te == NULL) {
    LOG(ERROR) << "MoverMessageProcessor: Unable to create random timer.";
    sendGoodBye();
  }

}

MoverMessageProcessor::~MoverMessageProcessor() {
};

int MMPTimerEvent::weakDelete()  {
  if (mmp() != NULL) {
    mmp()->close();
    setmmp(NULL);
  }
  return TimerEvent::weakDelete();
}

}} //namespace
