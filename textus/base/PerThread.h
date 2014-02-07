/* PerThread.h -*- C++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Like thread local, but can be accessed from other threads.  Useful
 * when every thread might be slightly different, but everything needs
 * to be accessable from all threads.
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
