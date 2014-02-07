/* LineReaderEvent.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to read a file one line at a time.
 *
 */

#include "LineReaderEvent.h"

#include <string>

namespace textus { namespace file {

using namespace std;

LineReaderEvent::LineReaderEvent(LineReader *lr):Event(lr), eof_flag(false) {}

LineReader *LineReaderEvent::lineReader() {
  return dynamic_cast<LineReader *>(eventTarget());
}

void LineReaderEvent::setLineReader(LineReader *lr) {
  setEventTarget(lr);
}

void LineReaderEvent::do_it() {
  Synchronized(this);
  LineReader *parent = lineReader();
  if (parent) {
    if (eof_flag) {
      parent->setEof(true);
    } else {
      parent->lineRead(this);
    }
  }
}

}} //namespace
