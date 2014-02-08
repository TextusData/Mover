/* Mutex.h -*- c++ -*-
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

#ifndef TEXTUS_BASE_LOCKS_MUTEX_H_
#define TEXTUS_BASE_LOCKS_MUTEX_H_

#include <pthread.h>
#include <assert.h>
#include <string.h> // for memest

#include "textus/logging/Log.h"
#include "textus/base/Compiler.h"

namespace textus { namespace base { namespace locks {
class Mutex /* This is a lightweight class so we can build Base around it. */
{
  friend class Condition;

private:
  pthread_mutex_t mutex;

public:

  Mutex() {
    pthread_mutex_init(&mutex, NULL);
  }

  ~Mutex()
  {
    if (pthread_mutex_trylock(&mutex)) {
      LOG(ERROR) << "Mutex destructor called while locked: \n";
      LOG_CALL_STACK(ERROR);
      die();
    }
  }

  int lock() {
    int ret = pthread_mutex_lock(&mutex);
    assert (ret == 0);
    return ret;

  }

  int unlock() {
    mb();
    int ret = pthread_mutex_unlock(&mutex);
    assert (ret == 0);
    return ret;
  }

  int trylock() {
    mb();
    int ret = pthread_mutex_trylock(&mutex);
    return ret;
  }

  int cond_wait(pthread_cond_t *cond) {
    mb();
    int ret = pthread_cond_wait(cond, &mutex);
    assert (ret == 0);
    return ret;
  }
};

class Locker {
private:
  Mutex *m;

public:
  Locker(Mutex *mu) { m = mu; m->lock(); }
  ~Locker() { m->unlock(); }
};

}}} // namespace

#define LOCK(mutex) textus::base::locks::Locker UNIQUE_IDENTIFIER(LOCK) (mutex)

#endif // TEXTUS_BASE_LOCKS_MUTEX_H_
