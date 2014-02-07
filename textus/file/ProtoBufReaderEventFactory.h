/* ProtoBufReaderEventFactory.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
 *
 */

#ifndef TEXTUS_FILE_PROTOBUF_READER_EVENT_FACTORY_H_
#define TEXTUS_FILE_PROTOBUF_READER_EVENT_FACTORY_H_

#include "textus/event/EventFactory.h"
#include "textus/file/ProtoBufReader.h"
#include "textus/file/FileHandleEventFactory.h"

namespace textus { namespace file {

class ProtoBufReader;
class ProtoBufReaderEvent;

class ProtoBufReaderEventFactory: public textus::event::EventFactory {
private:
  AutoWeakDeref<ProtoBufReader> parent;

public:
  explicit ProtoBufReaderEventFactory(ProtoBufReader *lr): parent(lr) {}
  virtual ~ProtoBufReaderEventFactory() {}
  
  ProtoBufReader *protoBufReader() {
    Synchronized(this);
    return parent;
  }

  void setProtoBufReader(ProtoBufReader *lr) {
    Synchronized(this);
    parent = lr;
  }

  Event *protoBufEvent();
};

}} // namespace


#endif // TEXTUS_FILE_PROTOBUF_READER_EVENT_FACTORY_H_
