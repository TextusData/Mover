/* ProtoBufObject.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A class that represents a persistant object stored
 * in the file system in the form of a protocol buffer.
 *
 */

#ifndef TEXTUS_PERSIST_PROTO_BUF_OBJECT_H_
#define TEXTUS_PERSIST_PROTO_BUF_OBJECT_H_

#include "textus/base/Base.h"
#include "textus/persist/PersistantObject.h"
#include "textus/file/Directory.h"
#include "textus/file/FileHandle.h"
#include "textus/event/Message.h"
#include "textus/file/ProtoBufReader.h"

#include <string>

namespace textus { namespace persist {

using textus::event::MessageBuilder;
using textus::event::Message;
using textus::file::Directory;
using textus::file::FileHandle;
using textus::file::ProtoBufReader;
using std::string;

class ProtoBufObjectStore: public PersistantObjectStore {
private:
  AutoDeref<Directory> root;
  AutoDeref<MessageBuilder> builder;

public:
  ProtoBufObjectStore() {}
  virtual ~ProtoBufObjectStore() {};
  virtual PersistantObject *loadObject(string type, string locator);
  
  Directory *base() {
    Synchronized(this);
    return root;
  }

  void setBase(Directory *d) {
    Synchronized(this);
    root = d;
  }

};

class ProtoBufObject: public PersistantObject {
private:
  AutoDeref<Message> mess;
  AutoDeref<ProtoBufReader> pbr;

protected:
  friend class ProtoBufObjectStore;
  explicit ProtoBufObject();
  virtual int weakDelete();

public:
  static ProtoBufObject *loadObject(string locator);
  ProtoBufObject(Message *m, string locator, int mode = 0600);

  virtual ~ProtoBufObject();

  Message *message() {
    Synchronized(this);
    return mess;
  }

  void setMessage(Message *m) {
    Synchronized(this);
    mess = m;
    setValid();
  }

  void checkForMessage() {
    textus::file::ProtoBufReaderStatus::status stat;
    Message *m;
    m = pbr->readMessage(stat);
    if (stat == textus::file::ProtoBufReaderStatus::ok) {
      setMessage(m);
    }
  }

  ProtoBufReader *protoBufReader() {
    Synchronized(this);
    return pbr;
  }

  void setProtoBufReader(ProtoBufReader *p) {
    Synchronized(this);
    pbr = p;
  }

  virtual int tryReadLock() {
    Synchronized(this);
    return pbr->handle()->tryReadLock();
  }

  virtual int readLock()  {
    Synchronized(this);
    return pbr->handle()->readLock();
  }

  virtual int readUnlock() {
    Synchronized(this);
    return pbr->handle()->readUnlock();
  }

  virtual int tryWriteLock() {
    Synchronized(this);
    return pbr->handle()->tryWriteLock();
  }

  virtual int writeLock() {
    Synchronized(this);
    return pbr->handle()->writeLock();
  }

  virtual int writeUnlock() {
    Synchronized(this);
    return pbr->handle()->writeUnlock();
  }

  virtual int startFlush() {
    Synchronized(this);
    return pbr->handle()->flush();
  }

  virtual int update() {
    Synchronized(this);
    string d = mess->marshall();
    return pbr->handle()->write(d);
  }

};

}} // namespace


#endif // TEXTUS_PERSIST_PROTO_BUF_OBJECT_H_
