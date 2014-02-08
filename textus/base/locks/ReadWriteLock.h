/* ReadWriteLock.h -*- c++ -*-
 * Copyright (c) 2009-2010 Ross Biro
 * Actually implements the lock.
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

#ifndef TEXTUS_BASE_LOCKS_READWRITELOCK_H_
#define TEXTUS_BASE_LOCKS_READWRITELOCK_H_

#include "textus/base/Base.h"

#include <pthread.h>

namespace textus { namespace base { namespace locks {

// Only about 32K read threads can have a lock
// at once.  Any more will block.

class ReadWriteLock: virtual public textus::base::Base {
private:
  Mutex mutex;
  Condition condition;
  long long count;
  static const long long write_locked   = 0xF000000000000000ULL;
  static const long long write_unlocked = 0x0F00000000000000ULL;
  static const long long read_lock_bias = 0x0000000100010001ULL;
  static const long long read_lock_full = 0x0000400040004000ULL;
  int repairWriteLock();
  int repairReadLock();

protected:

  int checkReadLock()
  {
    if ((count & 0xFFFFFFFF) == ((count >> 16) & 0XFFFFFFFF)) {
      return 0;
    }
    return 1;
  }

  void readLockNoMutex()
  {
    while (writeLocked() || (count & 0x400040004000)) {
      condition.wait(mutex);
    }

    count += read_lock_bias;
  }

  void writeLockNoMutex()
  {
    while(locked()) {
      condition.wait(mutex);
    }
    count = write_locked;
  }

public:

  ReadWriteLock()
  {
    count = write_unlocked;
  }

  virtual ~ReadWriteLock()
  {
    {
      LOCK(&mutex);
      if (locked()) {
	LOG(ERROR) << "ReadWriteLock destructor called while locked: \n";
	LOG_CALL_STACK(ERROR);
	die();
      }
    }
  }

  bool readLocked()
  {
    return (count & ~(write_locked | write_unlocked)) != 0;
  }

  bool writeLocked()
  {
    if ((count & write_locked) == write_locked) {
      return true;
    } else if ((count & write_unlocked) == write_unlocked) {
      return false;
    }
    return repairWriteLock();
  }

  bool writeUnlocked() {
    return !writeLocked();
  }


  bool readUnlocked() {
    return !readLocked();
  }

  int locked()
  {
    return writeLocked() && readLocked();
  }

  int readTryLock() 
  {  
    // cheat and grab the mutex.  no one holds it long enough to matter.
    LOCK(&mutex);
    int ret = (writeUnlocked());
    if (ret)
      readLockNoMutex();
    return ret;
  }

  void readUnlock()
  {
    int need_signal = 0;
    LOCK(&mutex);
    if (count & read_lock_full) {
      need_signal = 1;
    }
    count -= read_lock_bias;
    if (count == 0 || need_signal) {
      condition.signal();
    } else if (checkReadLock()) {
      repairReadLock();
    }
  }

  int writeTryLock() {
    if (mutex.trylock()) {
      int ret = locked();
      if (!ret) {
	writeLockNoMutex();
      }
      mutex.unlock();
      return ret;
    }
    return 1;      
  }

  void writeUnlock() {
    LOCK(&mutex);
    count = write_unlocked;
    condition.signal();
  }

  void readLock() 
  {
    LOCK(&mutex);
    readLockNoMutex();
  }

  void writeLock()
  {
    LOCK(&mutex);
    writeLockNoMutex();
  }

};
}}} // namespace

#endif //TEXTUS_BASE_LOCKS_READWRITELOCK_H_
