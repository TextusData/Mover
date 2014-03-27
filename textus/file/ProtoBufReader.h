/* ProtoBufReader.h -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * A class to read one or more protobufs from a file.
 *
 * XXXXXXXXXXXX FIXME: I don't think this generates events properly.
 * Write a unit test.
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

#ifndef TEXTUS_FILE_PROTOBUF_READER_H_
#define TEXTUS_FILE_PROTOBUF_READER_H_

#include "textus/base/Base.h"
#include "textus/base/AutoDeref.h"
#include "textus/file/FileHandle.h"
#include "textus/file/ProtoBufReaderEventFactory.h"
#include "textus/file/ProtoBufReaderEvent.h"

#include <string>

namespace textus { namespace file {
using namespace std;
class ProtoBufReaderEvent;
class ProtoBufReaderEventFactory;
class InternalProtoBufReaderEvent;

class ProtoBufReaderStatus {
public:
  enum status {
  ok = 0,
  eof,
  wait,
  };
};

class ProtoBufReader: public EventTarget {
private:
  string buffer;
  AutoDeref<FileHandle> fh;
  AutoDeref<Error> err;
  AutoDeref<ProtoBufReaderEventFactory> factory;
  bool eof_flag;
  Thread *eof_thread;
  Thread *read_thread;

protected:
  virtual int weakDelete() {
    close();
    return Base::weakDelete();
  }

public:
  typedef ProtoBufReaderEvent EventType;
  typedef ProtoBufReaderEventFactory EventFactoryType;
  typedef textus::event::Message  *MessageType;
  typedef ProtoBufReaderStatus status;

  explicit ProtoBufReader(FileHandle *f);
  virtual ~ProtoBufReader() {
    assert(fh == NULL);
  }

  Message *readMessage(ProtoBufReaderStatus::status &stat);
  Message *waitForMessage();

  Error *error() {
    Synchronized(this);
    return err;
  }

  void setError(Error *e) {
    Synchronized(this);
    err = e;
    signal();
  }

  bool eof() {
    Synchronized(this);
    if (buffer.length()) {
      return false;
    }
    return eof_flag;
  }

  void setEof(bool e) {
    {
      Synchronized(this);
      eof_thread = Thread::self();
      eof_flag = e;
      signal();
    }
    if (e) {
      close();
    }
  }

  void setEventFactory(ProtoBufReaderEventFactory *lref);

  ProtoBufReaderEventFactory *eventFactory(void) {
    Synchronized(this);
    return factory;
  }

  void error(InternalProtoBufReaderEvent *);
  void read(InternalProtoBufReaderEvent *);
  void write(InternalProtoBufReaderEvent *);

  virtual void messageRead(ProtoBufReaderEvent *pbre) {}

  FileHandle *handle() {
    Synchronized(this);
    return fh;
  }

  virtual void close() {
    AUTODEREF(FileHandle *, close_me);
    {
      Synchronized(this);
      factory = NULL;
      if (fh) {
	close_me = fh;
	close_me->ref();
	fh = NULL;
      }
    }
    if (close_me) {
      close_me->setEventFactory(NULL);
      close_me->close();
    }
  }

};

}} // namespace

#endif // TEXTUS_FILE_PROTOBUF_READER_H_
