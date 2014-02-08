/* ThreadLocal.h -*- C++ -*-
 * Copyright (C) 2009-2013 Ross Biro
 *
 * A thread local template.  Creates a
 * thrad local variable of a given type.
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

#ifndef TEXTUS_BASE_THREAD_LOCAL_H_
#define TEXTUS_BASE_THREAD_LOCAL_H_

#include <pthread.h>
#include "textus/base/init/Init.h"
#include "textus/logging/Log.h"
#include "textus/base/CallBack.h"
#include "textus/base/Thread.h"
#include "textus/base/ReferenceList.h"

namespace textus { namespace base {

class ThreadLocalBase: virtual public Base {
protected:
  // Thunks  Could move this up to the template.
  virtual void cleanup() = 0;
  virtual void setWithVptr(void *vptr) = 0;

  static void static_cleanup(void *);
  static void updateValueCallBack(const CallBack *cb);
  static ReferenceList<ThreadLocalBase *> &locals();

public:
  ThreadLocalBase();
  virtual ~ThreadLocalBase();

  static void threadExit();

};

class ThreadLocalCallBack: virtual public ThreadCallBack {
private:
  ThreadLocalBase *t;
  void *data;

public:
  explicit ThreadLocalCallBack() {}
  virtual ~ThreadLocalCallBack() {}

  void setThreadLocal(ThreadLocalBase *th) {
    t = th;
  }

  ThreadLocalBase *getThreadLocal(void) const {
    return t;
  }
 
};


// Warning. Does not automatically clean up all the
// thread local variables on destruction.  The user
// must make sure that they have all been cleaned before
// this class is destructed.
template <class S> struct is_pointer { static const bool value=false; typedef S pointer_type; };
template <class S> struct is_pointer<S *> { static const bool value = true; typedef S pointer_type; };

template <class T> class ThreadLocal:public ThreadLocalBase {
private:
  pthread_key_t thread_key;

  void threadInit()
  {
    // Don't need to lock here because we are only messing
    // with thread local data.
  }

protected:
  virtual void cleanup();

  virtual void setWithVptr(void *vptr);

public:
  typedef typename is_pointer<T>::pointer_type pointer_type;

  ThreadLocal()
  {
    if (pthread_key_create(&thread_key, NULL)) {
      LOG(ERROR) << "Unable to create key for thread local storage.\n";
      LOG_CALL_STACK(ERROR);
      die();
    }
  }
    
  ~ThreadLocal() {
    pthread_key_delete(thread_key);
  }

  operator T () 
  {
    threadInit();
    return static_cast<T>(pthread_getspecific(thread_key));
  }

  T operator = (const T t) {
    threadInit();
    setWithVptr(reinterpret_cast<void *>(t));
    return t;
  }

  T operator -> () {
    threadInit();
    return static_cast<T>(pthread_getspecific(thread_key));
  }
  
  pointer_type &operator * () {
    threadInit();
    return *static_cast<T>(pthread_getspecific(thread_key));
  }

  T operator++();
  T operator++(int);


};

// prefix.
template <> inline int ThreadLocal<int>::operator ++() {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  l++;
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l));
  return l;
}

// postfix.
template <> inline int ThreadLocal<int>::operator ++(int dummy) {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l+1));
  return l;
}

//prefix
template <> inline uint64_t ThreadLocal<uint64_t>::operator ++() {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  l++;
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l));
  return l;
}

// postfix.
template <> inline uint64_t ThreadLocal<uint64_t>::operator ++(int dummy) {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l+1));
  return l;
}

// prefix.
template <> inline unsigned int ThreadLocal<unsigned int>::operator ++() {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  l++;
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l));
  return l;
}

// postfix.
template <> inline unsigned int ThreadLocal<unsigned int>::operator ++(int dummy) {
  threadInit();
  long l = reinterpret_cast<long>(pthread_getspecific(thread_key));
  pthread_setspecific(thread_key, reinterpret_cast<void *>(l+1));
  return l;
}

template <> inline ThreadLocal<int>::operator int() {
    threadInit();
    return reinterpret_cast<long>(pthread_getspecific(thread_key));
}

template <> inline ThreadLocal<unsigned int>::operator unsigned int() {
    threadInit();
    return reinterpret_cast<long>(pthread_getspecific(thread_key));
}

template <> inline ThreadLocal<uint64_t>::operator uint64_t() {
    threadInit();
    return reinterpret_cast<uint64_t>(pthread_getspecific(thread_key));
}

template <> inline ThreadLocal<bool>::operator bool() {
    threadInit();
    return reinterpret_cast<long>(pthread_getspecific(thread_key));
}


template <> inline void ThreadLocal<char *>::cleanup()
{
  void *v = pthread_getspecific(thread_key);
  pthread_setspecific(thread_key, NULL);
  char *cpp = static_cast<char *>(v);
  free(cpp);
}

template <class T> inline void ThreadLocal<T>::setWithVptr(void *vptr) {
  threadInit();
  pthread_setspecific(thread_key, vptr);
}

template <class T> inline void ThreadLocal<T>::cleanup()
{
  void *v = pthread_getspecific(thread_key);
  pthread_setspecific(thread_key, NULL);
  is_pointer<T> ip;
  if ( ip.value ) {
    T *t = static_cast< T *>(v);
    delete t;
  }
}


}} // namespace

#endif // TEXTUS_BASE_THREAD_LOCAL_H_
