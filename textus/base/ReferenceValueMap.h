/* ReferenceValueMap.h -*- c++ -*-
 * Copyright (c) 2010, 2014 Ross Biro
 *
 * A quick set extension that assume all
 * it's values are of type Base * and
 * then ref's and derefs them as appropriate.
 */

#ifndef TEXTUS_BASE_REFERENCE_VALUE_MAP_H_
#define TEXTUS_BASE_REFERENCE_VALUE_MAP_H_

#include "textus/base/Base.h"
#include "textus/base/Utility.h"
#include "textus/base/ReferenceSet.h"
#include "textus/base/ReferenceList.h"

#include <map>

namespace textus { namespace base {
using namespace std;

template <class kt, class T> class ReferenceValueMap;

template <class kt, class T> class ReferenceValueMap<kt, T *>: virtual public Base {
private:
  typedef AutoDeref<T> dt;
  map<kt, dt> base_map;

public:
  typedef typename map<kt, dt>::key_type key_type;
  typedef typename map<kt, dt>::mapped_type mapped_type;
  typedef typename map<kt, dt>::value_type value_type;
  typedef typename map<kt, dt>::key_compare key_compare;
  typedef typename map<kt, dt>::value_compare value_compare;
  typedef typename map<kt, dt>::pointer pointer;
  typedef typename map<kt, dt>::reference reference;
  typedef typename map<kt, dt>::const_reference const_reference;
  typedef typename map<kt, dt>::size_type size_type;
  typedef typename map<kt, dt>::difference_type difference_type;
  typedef typename map<kt, dt>::iterator iterator;
  typedef typename map<kt, dt>::const_iterator const_iterator;
  typedef typename map<kt, dt>::reverse_iterator reverse_iterator;
  typedef typename map<kt, dt>::const_reverse_iterator const_reverse_iterator;

  ReferenceValueMap(): base_map() {}
  ReferenceValueMap(const key_compare& comp): base_map(comp) {}

  template <class InputIterator> ReferenceValueMap(InputIterator f, InputIterator l): base_map(f, l) {}
  template <class InputIterator> ReferenceValueMap(InputIterator f, InputIterator l, const key_compare& comp): base_map(f,l, comp) {}

  virtual ~ReferenceValueMap() { clear(); }

  iterator begin() {
    Synchronized(this);
    return base_map.begin();
  }

  iterator end() {
    Synchronized(this);
    return base_map.end();
  }

  const_iterator begin() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.begin();
  }

  const_iterator end() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.end();
  }

  reverse_iterator rbegin() {
    Syncrhonized(this);
    return base_map.rbegin();
  }

  reverse_iterator rend() {
    Syncrhonized(this);
    return base_map.rend();
  }

  const_reverse_iterator rbegin() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.rbegin();
  }

  const_reverse_iterator rend() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.rend();
  }

  size_type size() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.size();
  }

  size_type max_size() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.max_size();
  }

  bool empty() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.empty();
  }

  key_compare key_comp() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.key_comp();
  }

  value_compare value_comp() const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.value_comp();
  }


  void swap(ReferenceValueMap& m2) {
    Synchronized(this);
    Synchronized(&m2);
    base_map.swap(m2);
  }

  pair<iterator, bool> insert(const value_type& x) {
    Synchronized(this);
    return base_map.insert(x);
  }

  iterator insert(iterator pos, const value_type& x) {
    Synchronized(this);
    return base_map.insert(pos, x);
  }

  template <class InputIterator> void insert(InputIterator start, InputIterator end) {
    Synchronized(this);
    for (; start != end; ++start) {
      base_map.insert(*start);
    }
  }

  void erase(iterator pos) {
    Synchronized(this);
    base_map.erase(pos);
  }

  size_type erase(const key_type& k) {
    size_type c=0;
    Synchronized(this);
    for (iterator i = find(k); i != end(); i = find(k)) {
      erase(i);
      ++c;
    }
    return c;
  }

  void erase(iterator first, iterator last) {
    Synchronized(this);
    for (; first != last; ++first) {
      erase(first);
    }
  }

  void clear() {
    Synchronized(this);
    erase(begin(), end());
  }

  iterator find(const key_type& k) {
    Synchronized(this);
    return base_map.find(k);
  }

  const_iterator find(const key_type& k) const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.find(k);
  }

  size_type count(const key_type& k) {
    Synchronized(this);
    return base_map.count(k);
  }

  iterator lower_bound(const key_type& k) {
    Synchronized(this);
    return base_map.lower_bound(k);
  }

  const_iterator lower_bound(const key_type& k) const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.lower_bound(k);
  }

  iterator upper_bound(const key_type& k) {
    Synchronized(this);
    return base_map.upper_bound(k);
  }

  const_iterator upper_bound(const key_type& k) const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.upper_bound(k);
  }

  pair<iterator, iterator> equal_range(const key_type& k) {
    Syncrhonized(this);
    return base_map.equal_range(k);
  }

  pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map.equal_range(k);
  }

  mapped_type& operator[](const key_type& k) {
    Synchronized(this);
    return base_map[k];
  }

  bool operator==(const ReferenceValueMap& m2) const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    Synchronized(const_cast<ReferenceValueMap *>(&m2));
    return base_map == m2.base_map;
  }


  bool operator<(const ReferenceValueMap& m2)  const {
    Synchronized(const_cast<ReferenceValueMap *>(this));
    return base_map < m2.base_map;
  }

  ReferenceSet<T *> *valueSet() {
    ReferenceSet<T *> *set;
    set = new  ReferenceSet<T *>();
    if (set == NULL) {
      return NULL;
    }
    return set;
  }

  ReferenceList<T *> *valueList() {
    ReferenceList<T *> *list;
    list = new  ReferenceList<T *>();
    if (list == NULL) {
      return NULL;
    }
    Synchronizerd(this);
    for (iterator i = begin(); i != end(); ++i) {
      list->insert(i->second);
    }
    return list;
  }

  ReferenceSet<kt> *keySet() {
    ReferenceSet<kt> *set;
    set = new  ReferenceSet<kt>();
    if (set == NULL) {
      return NULL;
    }
    Synchronizerd(this);
    for (iterator i = begin(); i != end(); ++i) {
      set->insert(i->first);
    }
    return set;
  }
      

  COMPARISON_OPERATORS(ReferenceValueMap);
  

};

}} //namespace

#endif // TEXTUS_BASE_REFERENCE_VALUE_MAP_H_
