/* PersistantObject.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A class to represent a persistant object.  Persistant objects are
 * non-cache coherent, so effort must be taken to verify any updates
 * are handled properly.  Also there is always the potential for net
 * splits, so merges are inevitable as each side of the split updates
 * the object.
 *
 */

#ifndef TEXTUS_PERSIST_PERSISTANT_OBJECT_H_
#define TEXTUS_PERSIST_PERSISTANT_OBJECT_H_

#include "textus/base/Base.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/event/EventWatcher.h"

#include <string>
#include <map>

namespace textus { namespace persist {
using namespace textus::base;
using namespace std;

class PersistantObject;

class PersistantObjectStore: virtual public Base {
private:

public:
  PersistantObjectStore() {}
  virtual ~PersistantObjectStore() {}
  virtual PersistantObject *loadObject(string type, string locator) = 0;
};

class PersistantObject: public textus::event::EventTarget {
private:
  typedef ReferenceValueMap<string, PersistantObjectStore *> store_map;
  typedef store_map::iterator smIterator;

  static store_map objectStores;

  bool flush_in_progress;
  bool valid_flag;

protected:
  PersistantObject():flush_in_progress(false), valid_flag(false) {}

public:
  static void registerObjectStore(string type, PersistantObjectStore *pos);
  static void eraseObjectStore(string type);

  static PersistantObject *fetch(string type, string locator);
  virtual ~PersistantObject() {}

  bool flushInProgress() {
    Synchronized(this);
    return flush_in_progress;
  }

  void setFlushInProgress(bool f=true) {
    Synchronized(this);
    flush_in_progress = f;
    signal();
  }

  void waitForFlush() {
    Synchronized(this);
    while (flushInProgress()) {
      wait();
    }
  }

  void waitForValid() {
    Synchronized(this);
    while(!valid()) {
      wait();
    }
  }

  bool valid() {
    Synchronized(this);
    return valid_flag;
  }

  void setValid(bool f=true) {
    Synchronized(this);
    valid_flag = f;
    signal();
  }
  
  virtual int tryReadLock() = 0;
  virtual int readLock() = 0;
  virtual int readUnlock() = 0;

  virtual int tryWriteLock() = 0;
  virtual int writeLock() = 0;
  virtual int writeUnlock() = 0;
  virtual int startFlush() = 0;

  virtual int update() = 0;
  
};


}} //namespace


#endif // TEXTUS_PERSIST_PERSISTANT_OBJECT_H_
