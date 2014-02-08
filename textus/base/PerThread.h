/* PerThread.h -*- C++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Like thread local, but can be accessed from other threads.  Useful
 * when every thread might be slightly different, but everything needs
 * to be accessable from all threads.
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

#ifndef TEXTUS_BASE_PERTHREAD_H_
#define TEXTUS_BASE_PERTHREAD_H_

#include "Base.h"
#include "ThreadLocal.h"

#include <map>

namespace textus { namespace base {

template <typename T> class PerThread: public Base {
public:
  typedef std::map<ThreadID, T> perThreadMap;
  class iterator: public perThreadMap::iterator {

  public:
    iterator() {}
    explicit iterator(typename perThreadMap::iterator i): perThreadMap::iterator(i) {}
    virtual ~iterator() {}

    T operator *() {
      return *(operator ->());
    }

    T *operator ->() {
      return perThreadMap::iterator::operator *().second;
    }

  };

private:

protected:
  perThreadMap map;
  ThreadLocal<T> tl;

public:
  PerThread() {}
  virtual ~PerThread() {}
  
  iterator begin() {
    iterator i = iterator(map.begin());
    return i;
  }

  iterator end() {
    iterator i = iterator(map.end());
    return i;
  }

  operator T() {
    return tl;
  }

  T operator = (T t) {
    tl = t;
    map[ThreadID::self()] = t;
    return t;
  }

  T operator -> () {
    return tl.operator ->();
  }
  
  T operator * () {
    return tl.operator *();
  }
  
};

}}

#endif // TEXTUS_BASE_PERTHREAD_H_
