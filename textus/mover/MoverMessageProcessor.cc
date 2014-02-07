/* MoverMessageProcssor.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
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
  Synchronized(this);
  if (mmp() != NULL) {
    mmp()->timeOut();
  }
  close();
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
											    sending_messages(true) 
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
