/* Handle.h -*- c++ -*-
 * Copyright (c) 2009-2013 Ross Biro
 *
 * Represents a handle that can be used for i/o.
 * Generally if the handle has an underlying file descriptor,
 * FileHandle should be used instead.
 *
 */

#ifndef TEXTUS_FILE_HANDLE_H_
#define TEXTUS_FILE_HANDLE_H_

#include "textus/file/HandleEvent.h"
#include "textus/file/HandleEventFactory.h"
#include "textus/event/EventQueueScheduler.h"
#include "textus/event/Actor.h"
#include "textus/threads/WorkerThread.h"


namespace textus { namespace file {

class Handle: public TextusFile {
private:
  AutoDeref<HandleEventFactory> factory;
  AutoDeref<EventQueueScheduler> scheduler;
  AutoDeref<Actor> context_obj; // an object that represnts why this file was opened and what should be done with it.

protected:
  virtual void sawEOF() = 0;
  virtual void error() = 0;
  virtual void spaceAvailable() = 0;
  virtual void dataAvailable() = 0;

  bool eof_flag;
  bool error_flag;
  int err;

public:
  Handle(): eof_flag(false), error_flag(false), err(-1) {}
  virtual ~Handle() {}

  virtual string read(int maxlen) = 0;
  virtual int write(string data) = 0;
  virtual void close() = 0;
  
  Actor *context() {
    Synchronized(this);
    return context_obj;
  }

  void setContext(Actor *c) {
    Synchronized(this);
    context_obj = c;
  }

  virtual HandleEventFactory *eventFactory() {
    Synchronized(this);
    return factory;
  }

  virtual void setEventFactory(HandleEventFactory *f) {
    Synchronized(this);
    factory = f;
  }

  EventQueueScheduler *eventQueueScheduler() {
    Synchronized(this);
    return scheduler;
  }

  void setEventQueueScheduler(EventQueueScheduler *eqs) {
    Synchronized(this);
    scheduler = eqs;
  }

  textus::event::EventQueue *eventQueue() {
    Synchronized(this);
    if (scheduler) {
      return scheduler->eventQueue();
    }      
    return textus::threads::WorkerThread::workerThreadScheduler()->eventQueue();
  }

  void setEventQueue(textus::event::EventQueue *eq) {
    Synchronized(this);
    if (scheduler) {
      scheduler->setEventQueue(eq);
    }
  }

  bool hadError() {
    Synchronized(this);
    return error_flag;
  }

  void clearError() {
    Synchronized(this);
    error_flag = false;
  }

  void setError() {
    Synchronized(this);
    error_flag = true;
  }

  virtual bool eof() {
    Synchronized(this);
    return eof_flag;
  }

  virtual void setEof() {
    Synchronized(this);
    eof_flag = true;
  }

  void setError(int e) {
    Synchronized(this);
    err = e;
  }

};

}} //namespace


#endif // TEXTUS_FILE_HANDLE_H_
