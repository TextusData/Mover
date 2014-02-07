/* LineReaderEventFactory.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
 *
 */

#ifndef TEXTUS_FILE_LINE_READER_EVENT_FACTORY_H_
#define TEXTUS_FILE_LINE_READER_EVENT_FACTORY_H_

#include "textus/event/EventFactory.h"
#include "textus/file/LineReader.h"
#include "textus/file/FileHandleEventFactory.h"

namespace textus { namespace file {

class LineReader;
class LineReaderEvent;

class LineReaderEventFactory: public textus::event::EventFactory {
private:
  AutoWeakDeref<LineReader> parent;

public:
  explicit LineReaderEventFactory(LineReader *lr): parent(lr) {}
  virtual ~LineReaderEventFactory() {}
  
  LineReader *lineReader() {
    Synchronized(this);
    return parent;
  }

  void setLineReader(LineReader *lr) {
    Synchronized(this);
    parent = lr;
  }

  Event *lineEvent();
};

}} // namespace


#endif // TEXTUS_FILE_LINE_READER_EVENT_FACTORY_H_
