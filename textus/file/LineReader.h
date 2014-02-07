/* LineReader.h -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * A class to read a file one line at a time.
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
    if (fh) {
      fh->setEventFactory(NULL);
      fh = NULL;
    }
    factory = NULL;
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
