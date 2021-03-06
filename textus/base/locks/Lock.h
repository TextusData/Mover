/* Lock.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * A wrapper around the pthreads mutex class.
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

#ifndef TEXTUS_BASE_LOCKS_LOCK_H_
#define TEXTUS_BASE_LOCKS_LOCK_H_

#include "textus/base/Base.h"
#include "textus/base/locks/Mutex.h"

namespace textus { namespace base { namespace locks {
  class Lock /* Lightweight class, no base. */ {
  private:
    class Mutex *mutex;

  public:
    Lock(Mutex *m):mutex(m)
    {
      mutex->lock();
    }

    virtual ~Lock()
    {
      mutex->unlock();
    }
  };

}}} // namespace

  // need using textus::base:locks; to use this.  It creates a lock
  // that is automatically released at the end of the current
  // scope. name is just a name so we can have more than one in
  // nested scopes with out risking compiler warnings.  It has nothing
  // to do with any variables unlike the similiar locking mechanism in
  // java.
#define SYNCHRONIZE_INTERNAL1(name) static Mutex name ## _synchronize_mutex; \
  Lock name ## _synchronize_lock (& name ## _synchronize_mutex );
#define SYNCHRONIZE_INTERNAL(name) SYNCHRONIZE_INTERNAL1(name)
#define SYNCH() SYNCHRONIZE_INTERNAL(UNIQUE_IDENTIFIER(SYNC))

#endif // TEXTUS_BASE_LOCKS_LOCK_H_
