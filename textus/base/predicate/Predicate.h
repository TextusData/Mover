/* Predicate.h -*- C++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * A Class the represents a predicate.
 * so it can be applied to objects in a collection.
 */

#ifndef TEXTUS_BASE_PREDICATE_H_
#define TEXTUS_BASE_PREDICATE_H_

namespace textus { namespace base { namespace predicate {

class Predicate: virtual public Base {
public:
  Predicate() {}
  virtual ~Predicate() {}
  
  virtual bool test(const Base *) = 0;
};

// really a namespace, but we build it 
// so we can reuse the code all over the
// place.  The type is the container type.
template <typename T> class Filters {
 public:
  void removeMatching(T *t, Predicate *p) {
    AUTODEREF (T *, nt);
    nt = pullOut(t, p);
    nt->clear();
  }

  void keepMatching(T *t, Predicate *p) {
    AUTODEREF (T *, nt);
    nt = leaveIn(t, p);
    nt->clear();
  }

  T *filterMatching(T *t, Predicate *p) {
    T *nt = new T();
    for (typename T::iterator i = t->begin(); i != t->end(); ++i) {
      if (p->test(*i)) {
	nt->push_back(*i);
      }
    }
    return nt;
  }

  T *pullOut(T *t, Predicate *p) {
    T *nt = new  T();
    if (nt == NULL) {
      return nt;
    }

    typename T::iterator n;
    for (typename T::iterator i = t->begin(); i != t->end(); i = n) {
      n = i;
      ++n;
      if (p->test(*i)) {
	nt->append(*i);
	t->erase(i);
      }
    }
    return nt;
  }


  T *leaveIn(T *t, Predicate *p) {
    T *nt = new  T();
    if (nt == NULL) {
      return nt;
    }

    typename T::iterator n;
    for (typename T::iterator i = t->begin(); i != t->end(); i = n) {
      n = i;
      ++n;
      if (!p->test(*i)) {
	nt->append(*i);
	t->erase(i);
      }
    }
    return nt;
  }


};

// example predicates
class IsNull: public Predicate {
public:
  IsNull() {}
  virtual ~IsNull() {}
  virtual bool test(const Base *b ) { return (b == NULL); }
};

class And: public Predicate {
  AutoDeref<Predicate> pred1, pred2;
public:
  And(Predicate *p1, Predicate *p2):pred1(p1), pred2(p2)  { }
  virtual ~And() {}
  virtual bool test(const Base *b) { return (pred1->test(b) && pred2->test(b)); }
};

class Or: public Predicate {
  AutoDeref<Predicate> pred1, pred2;
public:
  Or(Predicate *p1, Predicate *p2): pred1(p1), pred2(p2)  { }
  virtual ~Or() {}
  virtual bool test(const Base *b) { return (pred1->test(b) || pred2->test(b)); }
};

class Not: public Predicate {
  AutoDeref<Predicate> pred;
public:
  Not(Predicate *p) {
    pred = p;
  }
  virtual ~Not() {}
  virtual bool test(const Base *b) { return !pred->test(b); }
};

template <typename T> class IsA: public Predicate {
public:
  IsA() {}
  virtual ~IsA() {}
  virtual bool test(const Base *b) { return dynamic_cast<T *>(b) == NULL ? false : true; }
};



}}} //namespace


#endif // TEXTUS_BASE_PREDICATE_H_
