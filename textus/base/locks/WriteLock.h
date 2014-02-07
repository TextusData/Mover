/* WriteLock.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * Takes a writelock and releases it in
 * the destructor.
 *
 */

#include "textus/base/locks/ReadWriteLock.h"

namespace textus::base::locks {
  class WriteLock /* lightweight no base */ {
  private:
    ReadWriteLock *rwl;
  public:
    WriteLock(ReadWiteLock *lock):rwl(lock) {
      rwl->writeLock();
    }
   
    virtual ~WriteLock() {
      rwl->writeUnlock();
    }
      
  };
}

