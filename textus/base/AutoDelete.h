/* AutoDelete.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A template to automatically delete a pointer
 * when an object is deleted or goes out of scope.
 */
#ifndef TEXTUS_BASE_AUTODELETE_H_
#define TEXTUS_BASE_AUTODELETE_H_

#include "textus/base/AutoFoo.h"

namespace textus { namespace base {

template <class T> class AutoDelete
{
 private:
  T *ptr;

 public:
  AutoDelete(): ptr(NULL) {}
  explicit AutoDelete(T *t) { ptr = t; } 
  ~AutoDelete() { if (ptr) delete ptr; }
  
  operator const T *() const
  {
    return ptr;
  }

  operator T *() 
  {
    return ptr;
  }

  T &operator[] (int n) {
    return ptr[n];
  }

  T &operator *() {
    return *ptr;
  }

  T* operator->() {
    return ptr;
  }

  T *operator = (T *t) {
    if (t != ptr) {
      if (ptr) {
	delete ptr;
      }
      ptr = t;
    }
    return t;
  }
};

}} //namespace

#endif //TEXTUS_BASE_AUTODELETE_H_
