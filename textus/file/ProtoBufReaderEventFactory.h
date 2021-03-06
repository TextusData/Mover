/* ProtoBufReaderEventFactory.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
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
