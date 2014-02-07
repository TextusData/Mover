/* AutoFoo.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A template to automatically Foo a pointer
 * when an object is deleted or goes out of scope.
 */
#ifndef TEXTUS_BASE_AUTOFOO_H_
#define TEXTUS_BASE_AUTOFOO_H_

namespace textus { namespace base {

template <class T, void (*foo)(T *) > class AutoFoo {
 private:
  T *ptr;

 public:
  AutoFoo(): ptr(NULL) {}
  explicit AutoFoo(T *t) { ptr = t; } 
  ~AutoFoo() { if (ptr) foo(ptr); }
  
  operator T *() 
  {
    return ptr;
  }

  operator const T*() const
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
	foo(ptr);
      }
      ptr = t;
    }
    return t;
  }
};
}} //namespace

#endif //TEXTUS_BASE_AUTOFOO_H_
