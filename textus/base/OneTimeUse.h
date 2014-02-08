/* OneTimeUse.h -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 *
 * Template for one time use thingies.  Needs a lot of integer like
 * properties to work properly.
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

#ifndef TEXTUS_BASE_ONE_TIME_USE_H_
#define TEXTUS_BASE_ONE_TIME_USE_H_

#include "textus/base/Base.h"
#include "textus/base/init/Init.h"
#include "textus/base/ThreadLocal.h"
#include "textus/base/LeakyBucket.h"

namespace textus { namespace base { 

DECLARE_INT_ARG(one_time_chunk_default_reserve);

template <typename T, typename S=T> class OneTimeUse: public Base {
 private:
  T v; //global next value to give out access under sync.
  ThreadLocal<S> vmin, vmax; // the local reserved ones.
  LeakyBucket<int> bucket;

 protected:
  void addChunk(ThreadLocal<S> *min, ThreadLocal<S> *max);

  int reserve;

  void increaseChunkSize() {
    reserve = reserve*2; 
  }
    
  void reserveChunk() {
    Synchronized(this);
    if ((++bucket).overflow()) {
      increaseChunkSize();
    }
    addChunk(&vmin, &vmax);
  }

 public:
  explicit OneTimeUse():reserve(one_time_chunk_default_reserve) {}
  explicit OneTimeUse(T iv): v(iv), reserve(one_time_chunk_default_reserve) {}
  virtual ~OneTimeUse() {}

  operator S() {
    // can't give out vmax, not ours.
    // vmin and vmax are thread local, so no need to
    // synchronize.
    if (vmin >= vmax) {
      reserveChunk();
    }
    S ret = vmin++;
    return ret;
  }

  T &getBacking() {
    return v;
  }

  T getMaxPossible() {
    Synchronized(this);
    return v;
  }

};

template <class T, class S> 
inline void OneTimeUse<T, S>::addChunk(ThreadLocal<S> *min,
				       ThreadLocal<S> *max) {
  *min = v;
  for (int i = 0; i < reserve; ++i) {
    v++;
  }
  *max = v;
}

template <> inline void OneTimeUse<int, int>::addChunk(ThreadLocal<int> *min,
						       ThreadLocal<int> *max) {
  *min = v;
  v = v + reserve;
  *max = v;
}


}}


#endif // TEXTUS_BASE_ONE_TIME_USE_H_
