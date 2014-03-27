/* ReferenceList.h -*- c++ -*-
 * Copyright (c) 2009, 2014 Ross Biro
 *
 * A quick list extension that assume all
 * it's entries are of type Base * and
 * then ref's and derefs them as appropriate.
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

#ifndef TEXTUS_BASE_REFERENCE_LIST_H
#define TEXTUS_BASE_REFERENCE_LIST_H

#include "textus/base/Base.h"
#include "textus/base/Utility.h"

#include <list>

namespace textus { namespace base {
using namespace std;

#ifndef foreach
#define foreach(it, container) for (typeof((container).member_iterator) it = (container).begin(); it != (container).end(); ++it)
#endif

template <class value_type> class ReferenceList: virtual public Base {
 private:
  list<value_type> base_list; // Not an is a because list isn't list
  // up for inheritence

 public:
  typedef typename list<value_type>::iterator iterator;
  typedef typename list<value_type>::size_type size_type;
  typedef typename list<value_type>::reverse_iterator reverse_iterator;
  typedef typename list<value_type>::reference reference_type;

  static iterator member_iterator;

  ReferenceList(): base_list()
    {
    }

  virtual ~ReferenceList() { clear(); }

  iterator begin() { return base_list.begin(); }
  iterator end() { return base_list.end(); }
  reverse_iterator rbegin() { return base_list.rbegin(); }
  reverse_iterator rend() { return base_list.rend(); }
  size_type size() const { return base_list.size(); }
  size_type max_size() const { return base_list.max_size(); }
  bool empty() const { return base_list.empty(); }
  void sort() { base_list.sort(); }
  void unique() {
    iterator i;
    iterator l = begin();
    iterator n;
    for (i = begin(); i != end(); i = n) {
      n=i;
      n++;
      if (i == begin()) {
	l = i;
      } else {
	if (*l == *i) {
	  (*i)->deref();
	  base_list.erase(i);
	} else {
	  l = i;
	}
      }
    }
  }

  void swap(ReferenceList &s2) { base_list.swap(s2.base_list); }

  iterator find(const value_type &k) {  
    iterator i;
    for (i = begin(); i != end(); ++i) {
      if (*i == k) {
	break;
      }
    }
    return i;
  }
  iterator lower_bound(const value_type &k) { return base_list.lower_bound(k); }
  iterator upper_bound(const value_type &k) { return base_list.upper_bound(k); }

  pair<iterator, iterator> equal_range(const value_type& k) const { return base_list.equal_range(k); }

  friend bool operator==(const ReferenceList& s1, const ReferenceList& s2) { return s1.base_list == s2.base_list; }

  friend bool operator<(const ReferenceList& s1,const ReferenceList& s2) { return s1.base_list < s2.base_list; }

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
    t->ref();
    return base_list.push_front(t);
  }

  void push_back(const value_type&t)
  {
    Synchronized(this);
    t->ref();
    return base_list.push_back(t);
  }

  void pop_front()
  {
    Synchronized(this);
    base_list.front()->deref();
    base_list.pop_front();
  }
    
  void pop_back()
  {
    Synchronized(this);
    base_list.back()->deref();
    base_list.pop_back();
  }

  COMPARISON_OPERATORS(ReferenceList)

  iterator insert(const value_type& x)
  {
    SYNCHRONIZED(this);
    x.ref();
    return base_list.insert(x);
  }

  iterator insert(iterator pos, const value_type& x)
  {
    SYNCHRONIZED(this);
    x->ref();
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


  iterator  erase(iterator pos)
  {
    SYNCHRONIZED(this);
    value_type t = *pos;
    pos = base_list.erase(pos);
    t->deref();
    return pos;
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
    for (;first != last; first = erase(first)) {
    }
  }

  void clear() {
    SYNCHRONIZED(this);
    erase(base_list.begin(), base_list.end());
  }

  bool contains(const value_type &k) { 
    SYNCHRONIZED(this);
    iterator i = find(k);
    return i != end();
  }

};
}} //namespace


#endif //TEXTUS_BASE_REFERENCE_LIST_H
