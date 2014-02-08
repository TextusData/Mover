/* Functor.h -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * A Class the represents a predicate.
 * so it can be applied to objects in a collection.
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

#ifndef TEXTUS_BASE_FUNCTOR_FUNCTOR_H_
#define TEXTUS_BASE_FUNCTOR_FUNCTOR_H_

#include "textus/base/Base.h"
#include "textus/base/predicate/Predicate.h"

namespace textus { namespace base { namespace functor {

using namespace textus::base::predicate;

class Functor: virtual public Base {
public:
  Functor() {}
  virtual ~Functor() {}

  virtual Base *apply(Base *) = 0;
  Base *operator ()(Base *b) { return apply(b); }
};

class AccumulationFunctor: virtual public Functor {
public:
  AccumulationFunctor() {}
  virtual ~AccumulationFunctor() {}

  virtual Base *finalize() = 0; // Returns what's been accumulated so far.
};

class BiFunctor: virtual public Base {
public:
  BiFunctor() {}
  virtual ~BiFunctor() {}
  virtual Base *apply(Base *b1, Base *b2) = 0;
  Base *operator()(Base *b1, Base *b2) { return apply(b1, b2); }
};

template <typename T> class RemoveDBCruft : public Functor {
 public:
  RemoveDBCruft() {}
  virtual ~RemoveDBCruft() {}
  
  Base *apply(Base *b) {
    T *t = dynamic_cast<T *>(b);
    return static_cast<Base *>(t->withoutCruft());
  }
};

// really a namespace
template <typename T> class Map {
public:
  static T *map(T *t, Predicate *p, Functor *f) {
    T *nt = new T();
    if (nt == NULL) {
      return nt;
    }
    for (typename T::iterator i = t->begin(); i != t->end(); ++i) {
      if (p == NULL || p->test(*i)) {
	nt->append(f->apply(*i));
      }
    }
    return nt;
  }

  static T *map(T *t1, T *t2, Predicate *p1, Predicate *p2, BiFunctor *f) {
    T *nt = new T();
    if (nt == NULL) {
      return nt;
    }
    typename T::iterator i2 = t2->begin();
    for (typename T::iterator i1 = t1->begin(); i1 != t1->end() && i2 != t2->end(); ++i1, ++i2) {
      if ((p1 == NULL || p1->test(*i1) && (p2 == NULL || p2->test(*i2)))) {
	nt->append(f->apply(*i1, *i2));
      }
    }
    return nt;
  }

  static void vmap(T *t, Predicate *p, Functor *f) {
    for (typename T::iterator i = t->begin(); i != t->end(); ++i) {
      if (p == NULL || p->test(*i)) {
	AUTODEREF(Base *, b);
	b = f->apply(*i);
      }
    }
  }
  
  static Base *reduce(T *t, Predicate *p, AccumulationFunctor *f) {
    // The functor accumulates the information internally.  
    // So the last base returned is the final one.
    // Each intermediate base will be derefed if it's not null.
    for (typename T::iterator i = t->begin(); i != t->end(); ++i) {
      if (p == NULL || p->test(*i)) {
	f->apply(*i);
      }
    }
    return f->finalize();
  }

  static Base *mapReduce(T *t, Predicate *reduce_filter, AccumulationFunctor *reduce_functor, Predicate *map_filter, Functor *map_functor) {
    AUTODEREF(T *, nt);
    for (typename T::iterator i = t->begin(); i != t->end(); ++i) {
      if (map_filter == NULL || map_filter(*i)) {
	AUTODEREF(Base *, b);
	b = map_functor->apply(*i);
	if (reduce_filter == NULL || reduce_filter->test(b)) {
	  reduce_functor->apply(b);
	}
      }
    }
    return reduce_functor->finalize();
  }
};

}}} //namespace

#endif // TEXTUS_BASE_FUNCTOR_FUNCTOR_H_

