/* AutoDeref.h -*- C++ -*-
 * Copyright (C) 2010-2014 Ross Biro
 *
 * A template to automatically deref an pointer
 * when an object is deleted or goes out of scope.
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

#ifndef TEXTUS_BASE_AUTODEREF_H_
#define TEXTUS_BASE_AUTODEREF_H_
namespace textus { namespace base {
class Base;

template <class T> class AutoDeref {
 private:
  T *ptr;

 public:
  AutoDeref(): ptr(NULL) {}
  explicit AutoDeref(T *t) { ptr = t; if (t) t->ref(); } 
  ~AutoDeref() { if (ptr) const_cast<T *>(static_cast<const T *>(ptr))->deref(); }
  
  operator const T*() const
  {
    return ptr;
  }

  operator T *() 
  {
    return ptr;
  }

  friend ostream & operator << (ostream &os, const AutoDeref<T> x) {
    return os << x.ptr;
  }

  //bool operator == (const T *t) const { return ptr == t; }

  //bool operator == (const AutoDeref<T> o) const { return o.ptr == ptr; }

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
	T *tmp;
	tmp = ptr;
	ptr = NULL;
	const_cast<textus::base::Base *>(static_cast<const textus::base::Base *>(tmp))->deref();
      }
      ptr = t;
      if (ptr) {
	const_cast<textus::base::Base *>(static_cast<const textus::base::Base *>(ptr))->ref();
      }
    }
    return t;
  }

  void clearNoDeref() {
    ptr = NULL;
  }
  
};

template <class T> class AutoWeakDeref {
 private:
  T *ptr;

 public:
  AutoWeakDeref(): ptr(NULL) {}
  explicit AutoWeakDeref(T *t) { ptr = t; if (t) t->weakRef(); } 
  ~AutoWeakDeref() { if (ptr) ptr->weakDeref(); }
  
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
	T *tmp;
	tmp = ptr;
	ptr = NULL;
	tmp->weakDeref();
      }
      ptr = t;
      if (ptr) {
	ptr->weakRef();
      }
    }
    return t;
  }

  void clearNoDeref() {
    ptr = NULL;
  }
  
};

}};


#endif // TEXTUS_BASE_AUTODEREF_H_
