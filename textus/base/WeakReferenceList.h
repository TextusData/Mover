/* WeakReferenceList.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * A quick list extension that assume all
 * it's entries are of type Base * and
 * then weak ref's and weak derefs them as appropriate.
 */

#ifndef TEXTUS_BASE_WEAK_REFERENCE_LIST_H
#define TEXTUS_BASE_WEAK_REFERENCE_LIST_H

#include "textus/base/Base.h"
#include "textus/base/Utility.h"

#include <list>

namespace textus { namespace base {
using namespace std;

template <class value_type> class WeakReferenceList: virtual public Base {
 private:
  list<value_type> base_list; // Not an is a because list isn't list
  // up for inheritence

 public:
  typedef typename list<value_type>::iterator iterator;
  typedef typename list<value_type>::size_type size_type;
  typedef typename list<value_type>::reverse_iterator reverse_iterator;
  typedef typename list<value_type>::reference reference_type;

  WeakReferenceList(): base_list()
    {
    }

  virtual ~WeakReferenceList() { clear(); }

  iterator begin() { Synchronized(this); iterator i= base_list.begin(); value_type t = *i; assert(t != NULL); return i;}
  iterator end() { Synchronized(this); return base_list.end(); }
  reverse_iterator rbegin() { Synchronized(this); return base_list.rbegin(); }
  reverse_iterator rend() { Synchronized(this); return base_list.rend(); }
  size_type size() const { return base_list.size(); }
  size_type max_size() const { return base_list.max_size(); }
  bool empty() const { return base_list.empty(); }

  void swap(WeakReferenceList &s2) { base_list.swap(s2.base_list); }

  iterator find(const value_type &k) {  return base_list.find(k); }
  size_type count(const value_type &k) { return base_list.count(k); }
  iterator lower_bound(const value_type &k) { return base_list.lower_bound(k); }
  iterator upper_bound(const value_type &k) { return base_list.upper_bound(k); }

  pair<iterator, iterator> equal_range(const value_type& k) const { return base_list.equal_range(k); }

  friend bool operator==(const WeakReferenceList& s1, const WeakReferenceList& s2) { return s1.base_list == s2.base_list; }

  friend bool operator<(const WeakReferenceList& s1,const WeakReferenceList& s2) { return s1.base_list < s2.base_list; }

  reference_type front()
  {
    Synchronized(this);
    return base_list.front();
  }

  reference_type back()
  {
    Synchronized(this);
    return base_list.back();
  }

    
  void push_front(const value_type &t)
  {
    Synchronized(this);
    t->weakRef();
    return base_list.push_front(t);
  }

  void push_back(const value_type&t)
  {
    Synchronized(this);
    t->weakRef();
    return base_list.push_back(t);
  }

  void pop_front()
  {
    Synchronized(this);
    base_list.front()->weakDeref();
    base_list.pop_front();
  }
    
  void pop_back()
  {
    Synchronized(this);
    *(base_list.back())->weakDeref();
    base_list.pop_back();
  }

  COMPARISON_OPERATORS(WeakReferenceList)

    iterator insert(const value_type& x)
    {
      SYNCHRONIZED(this);
      x.weakRef();
      return base_list.insert(x);
    }

  iterator insert(iterator pos, const value_type& x)
  {
    SYNCHRONIZED(this);
    x->weakRef();
    return base_list.insert(pos, x);
  }

  template <class InputIterator> void insert(InputIterator first, InputIterator last)
  {
    // XXXXX FIXME: Needs a compile time assert that InputIterator is an InputIterator.
    SYNCHRONIZED(this);
    for (; first != last; ++first) {
      insert(*first);
    }
    return first;
  }


  iterator erase(iterator pos)
  {
    SYNCHRONIZED(this);
    (*pos)->weakDeref();
    return base_list.erase(pos);
  }

  size_type erase(const value_type& k)
  {
    SYNCHRONIZED(this);
    size_type erased = 0;
    iterator it = begin();
    while (it != end()) {
      if (*it == k) {
	it = erase(it);
	++erased;
	continue;
      }
      ++it;
    }
    return erased;
  }

  void erase(iterator first, iterator last)
  {
    SYNCHRONIZED(this);
    for (;first != last; first=erase(first)) {
    }
  }

  void clear() {
    SYNCHRONIZED(this);
    erase(base_list.begin(), base_list.end());
  }


};
}} //namespace


#endif //TEXTUS_BASE_WEAK_REFERENCE_LIST_H
