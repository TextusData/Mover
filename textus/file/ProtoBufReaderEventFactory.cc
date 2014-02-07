/* ProtoBufReaderEventFactory.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
 *
 */

#include "textus/file/ProtoBufReaderEventFactory.h"
#include "textus/file/ProtoBufReaderEvent.h"

namespace textus { namespace file {

Event *ProtoBufReaderEventFactory::protoBufEvent() {
  Synchronized(this);
  ProtoBufReaderEvent *lre = new ProtoBufReaderEvent(parent);
  setEventQueue(lre);
  return lre;
};

}} //namespace
