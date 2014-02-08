/* ReferenceSet.h -*- c++ -*-
 * Copyright (c) 2009-2012 Ross Biro
 *
 * A quick set extension that assume all
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

#ifndef TEXTUS_BASE_REFERENCESET_H
#define TEXTUS_BASE_REFERENCESET_H

#include "textus/base/Base.h"
#include "textus/base/Utility.h"

#include <set>

#ifndef foreach
#define foreach(it, container) for (typeof(container.member_iterator) it = container.begin(); it != container.end(); ++it)
#endif

namespace textus { namespace base {
using namespace std;

template <class value_type> class ReferenceSet: virtual public Base {
private:
  set<value_type> base_set; // Not an is a because set isn't set up for
  // inheritence

public:
  typedef typename set<value_type>::size_type size_type;
  typedef typename set<value_type>::iterator iterator;
  typedef typename set<value_type>::reverse_iterator reverse_iterator;
  static iterator member_iterator;

  ReferenceSet(): base_set()
  {
  }

  virtual ~ReferenceSet() { clear(); }

  iterator begin() const { return base_set.begin(); }
  iterator end() const { return base_set.end(); }
  reverse_iterator rbegin() const { return base_set.rbegin(); }
  reverse_iterator rend() const { return base_set.rend(); }
  size_type size() const  { return base_set.size(); }
  size_type max_size() const  { return  base_set.max_size(); }
  bool empty() const  { return  base_set.empty(); }

  void swap(ReferenceSet &s2) { base_set.swap(s2.base_set); }

  iterator find(const value_type& k) {  return base_set.find(k); }
  size_type count(const value_type& k) { return base_set.count(k); }
  iterator lower_bound(const value_type& k) { return base_set.lower_bound(k); }
  iterator upper_bound(const value_type& k) { return base_set.upper_bound(k); }

  pair<iterator, iterator> equal_range(const value_type& k) const { return base_set.equal_range(k); }

  friend bool operator==(const ReferenceSet& s1, const ReferenceSet& s2) { return s1.base_set == s2.base_set; }

  friend bool operator<(const ReferenceSet& s1,const ReferenceSet& s2) { return s1.base_set < s2.base_set; }

  COMPARISON_OPERATORS(ReferenceSet)

  pair<iterator, bool> insert(const value_type& x)
  {
    Synchronized(this);
    pair<iterator, bool> p = base_set.insert(x);
    if (p.second) {
      x->ref();
    }
    return p;
  }

  iterator insert(iterator pos, const value_type& x)
  {
    Synchronized(this);
    size_type old_size = base_set.size();
    iterator i = base_set.insert(pos, x);
    if (old_size != base_set.size()) {
      x->ref();
    }
    return i;
  }

  template <class InputIterator> void insert(InputIterator first, InputIterator last)
  {
    // XXXXX FIXME: Needs a compile time assert that InputIterator is an InputIterator.
    Synchronized(this);

    for (; first != last; ++first) {
      insert(*first);
    }
    return first;
  }


  void erase(iterator pos)
  {
    Synchronized(this);
    (*pos)->deref();
    base_set.erase(pos);
  }

  size_type erase(const value_type& k)
  {
    Synchronized(this);
    size_type erased = base_set.erase(k);
    for (size_type s = erased; s > 0; s--) {
      k->deref();
    }
    return erased;
  }

  void erase(iterator first, iterator last)
  {
    Synchronized(this);
    for (;first != last; ++first) {
      erase(first);
    }
  }

  void clear() {
    Synchronized(this);
    erase(base_set.begin(), base_set.end());
  }


};
}} //namespace


#endif //TEXTUS_BASE_REFERENCESET_H
