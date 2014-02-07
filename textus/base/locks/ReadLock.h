/* ReadLock.h -*- c++ -*-
 * Copyright (c) 2009-2010 Ross Biro
 * Takes a readlock and releases it in
 * the destructor.
 *
 */

#include "textus/base/locks/ReadWriteLock.h"

#ifndef TEXTUS_BASE_LOCKS_READ_LOCK_H_
#define TEXTUS_BASE_LOCKS_READ_LOCK_H_

namespace textus { namespace base { namespace locks {
class ReadLock /* LightWeight, no Base. */ {
private:
  ReadWriteLock *rwl;
public:
  explicit ReadLock(ReadWriteLock *lck):
    rwl(lck)
  {
    rwl->readLock();
  }
   
  virtual ~ReadLock() {
    rwl->readUnlock();
  }
      
};

class WriteLock /* LightWeight, no Base. */ {
private:
  ReadWriteLock *rwl;
public:
  explicit WriteLock(ReadWriteLock *lck):
    rwl(lck)
  {
    rwl->writeLock();
  }
   
  virtual ~WriteLock() {
    rwl->writeUnlock();
  }
      
};


}}} //namespace


#endif //TEXTUS_BASE_LOCKS_READ_LOCK_H_
