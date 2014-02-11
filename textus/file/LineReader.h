/* LineReader.h -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * A class to read a file one line at a time.
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

#ifndef TEXTUS_FILE_LINE_READER_H_
#define TEXTUS_FILE_LINE_READER_H_

#include "textus/base/Base.h"
#include "textus/base/AutoDeref.h"
#include "textus/file/FileHandle.h"
#include "textus/file/LineReaderEventFactory.h"
#include "textus/file/LineReaderEvent.h"

#include <string>

namespace textus { namespace file {
using namespace std;
class LineReaderEvent;
class LineReaderEventFactory;
class InternalLineReaderEvent;

class LineReaderStatus {
public:
  enum status {
  ok = 0,
  eof,
  wait,
  };
};

class LineReader: public EventTarget {
private:
  string buffer;
  AutoDeref<FileHandle> fh;
  AutoDeref<Error> err;
  AutoDeref<LineReaderEventFactory> factory;
  bool eof_flag;

protected:
  virtual int weakDelete() {
    close();
    return EventTarget::weakDelete();
  }

public:
  typedef LineReaderEvent EventType;
  typedef LineReaderEventFactory EventFactoryType;
  typedef std::string MessageType;

  explicit LineReader(FileHandle *f);
  virtual ~LineReader() {
    assert(fh == NULL);
  }

  virtual void close() {
    Synchronized(this);
    if (fh) {
      AUTODEREF(FileHandle *, tmp);
      tmp = fh;
      tmp->ref();
      fh = NULL;
      tmp->setEventFactory(NULL);
      tmp->close();
    }
    factory = NULL;
  }

  string readLine(LineReaderStatus::status &stat);
  string waitForLine();

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
    Synchronized(this);
    eof_flag = e;
    signal();
  }

  void setEventFactory(LineReaderEventFactory *lref);

  LineReaderEventFactory *eventFactory(void) {
    Synchronized(this);
    return factory;
  }

  void error(InternalLineReaderEvent *);
  void read(InternalLineReaderEvent *);
  void setEof();

  virtual void lineRead(LineReaderEvent *lre) {}

  FileHandle *handle() {
    Synchronized(this);
    return fh;
  }

};

}} // namespace

#endif // TEXTUS_FILE_LINE_READER_H_
