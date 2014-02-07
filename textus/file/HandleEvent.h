/* HandleEvent.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Base class for events related to files.
 * Handles a bunch of common cases.
 *
 */

#ifndef TEXTUS_FILE_HANDLE_EVENT_H_
#define TEXTUS_FILE_HANDLE_EVENT_H_

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/base/AutoDeref.h"
#include "textus/event/EventWatcher.h"
#include "textus/base/Error.h"
#include "textus/file/TextusFile.h"

namespace textus { namespace file {

using namespace textus::base;

class HandleEvent: public textus::event::Event {
private:
  AutoDeref<Error> err;
  string buffer;
  bool write;
  bool eof_flag;

public:
  HandleEvent(textus::event::EventTarget *t): Event(t), write(false), eof_flag(false) {
    return;
  }
  virtual ~HandleEvent() {}

  virtual Error *error() {
    Synchronized(this);
    return err;
  }

  virtual void setError(Error *e) {
    Synchronized(this);
    err = e;
  }

  virtual bool canWrite() const {
    return write;
  }

  virtual void setCanWrite(bool yes) {
    Synchronized(this);
    write = yes;
  }

  virtual string data() {
    Synchronized(this);
    return buffer;
  }

  virtual void setData(string d) {
    Synchronized(this);
    buffer = d;
  }

  virtual bool eof() const {
    SynchronizedConst(this);
    return eof_flag;
  }

  void setEof(bool f) {
    Synchronized(this);
    eof_flag = f;
  }

};

}} //namespace

#endif // TEXTUS_FILE_HANDLE_EVENT_H_
