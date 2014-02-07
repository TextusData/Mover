/* AutoFree.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A template to automatically free a pointer
 * when an object is deleted or goes out of scope.
 */
#ifndef TEXTUS_BASE_AUTOFREE_H_
#define TEXTUS_BASE_AUTOFREE_H_

#include "textus/base/AutoFoo.h"

namespace textus { namespace base {

template <class T> class AutoFree
{
 private:
  T *ptr;

 public:
  AutoFree(): ptr(NULL) {}
  explicit AutoFree(T *t) { ptr = t; } 
  ~AutoFree() { if (ptr) free(ptr); }
  
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
	free(ptr);
      }
      ptr = t;
    }
    return t;
  }
};

}} //namespace

#endif //TEXTUS_BASE_AUTOFREE_H_
