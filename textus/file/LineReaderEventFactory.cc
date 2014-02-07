/* LineReaderEventFactory.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
 *
 */

#include "textus/file/LineReaderEventFactory.h"
#include "textus/file/LineReaderEvent.h"

namespace textus { namespace file {

Event *LineReaderEventFactory::lineEvent() {
  Synchronized(this);
  LineReaderEvent *lre = new LineReaderEvent(parent);
  setEventQueue(lre);
  return lre;
};

}} //namespace
