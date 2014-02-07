/* ProtoBufReaderEvent.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to read a file one message at a time.
 *
 */

#include "ProtoBufReaderEvent.h"

#include <string>

namespace textus { namespace file {

using namespace std;

ProtoBufReaderEvent::ProtoBufReaderEvent(ProtoBufReader *lr):Event(lr), eof_flag(false) {}

ProtoBufReader *ProtoBufReaderEvent::messageReader() {
  return dynamic_cast<ProtoBufReader *>(eventTarget());
}

void ProtoBufReaderEvent::setProtoBufReader(ProtoBufReader *lr) {
  setEventTarget(lr);
}

void ProtoBufReaderEvent::do_it() {
  Synchronized(this);
  ProtoBufReader *parent = messageReader();
  if (parent) {
    if (eof_flag) {
      parent->setEof(true);
    } else {
      parent->messageRead(this);
    }
  }
}

}} //namespace
