/* ProtoBufReaderEvent.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * A class to read a file one message at a time.
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

#ifndef TEXTUS_FILE_PROTOBUF_READER_EVENT_H_
#define TEXTUS_FILE_PROTOBUF_READER_EVENT_H_

#include "textus/base/Base.h"
#include "textus/file/ProtoBufReader.h"
#include "textus/event/Event.h"
#include "textus/event/Message.h"

namespace textus { namespace file {

class ProtoBufReader;

class ProtoBufReaderEvent: public textus::event::Event {
private:
  bool eof_flag;
  AutoDeref<Message> mess;
  AutoDeref<Error> err;

protected:
  virtual int weakDelete() {
    mess = NULL;
    err = NULL;
    return Event::weakDelete();
  }
  void do_it();

public:
  explicit ProtoBufReaderEvent(ProtoBufReader *lr);
  virtual ~ProtoBufReaderEvent() {}

  ProtoBufReader *messageReader();

  void setProtoBufReader(ProtoBufReader *lr);

  bool eof() {
    Synchronized(this);
    return eof_flag;
  }

  void setEof(bool e) {
    Synchronized(this);
    eof_flag = e;
  }

  void eof(ProtoBufReaderEvent *lre) {}

  Error *error() {
    Synchronized(this);
    return err;
  }

  void setError(Error *e) {
    Synchronized(this);
    err = e;
  }

  Message *message() {
    Synchronized(this);
    return mess;
  }

  void setMessage(Message *m) {
    Synchronized(this);
    mess = m;
  }

};

}} // namespace

#endif // TEXTUS_FILE_PROTOBUF_READER_EVENT_H_
