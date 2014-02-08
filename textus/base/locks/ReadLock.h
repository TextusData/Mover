/* ReadLock.h -*- c++ -*-
 * Copyright (c) 2009-2010 Ross Biro
 * Takes a readlock and releases it in
 * the destructor.
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
