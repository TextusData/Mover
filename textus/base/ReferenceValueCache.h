/* ReferenceValueCache.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 *
 * A template that represents a cache where the values are refed and derefed as appropriate.
 *
 */

#ifndef TEXTUS_BASE_REFERENCE_VALUE_CACHE_H_
#define TEXTUS_BASE_REFERENCE_VALUE_CACHE_H_

#include "textus/base/Cache.h"
#include "textus/base/Base.h"

namespace textus { namespace base {
template <class kt, class d0, size_t mct=20> class ReferenceValueCache;

template <class kt, class d1, size_t mct> class ReferenceValueCache<kt, d1 *, mct>: public virtual Base {
private:
  typedef AutoDeref<d1> dt;
  Cache<kt, dt, mct> base_cache;

public:
  typedef typename Cache<kt, dt>::key_type key_type;
  typedef typename Cache<kt, dt>::mapped_type mapped_type;
  typedef typename Cache<kt, dt>::value_type value_type;
  typedef typename Cache<kt, dt>::key_compare key_compare;
  typedef typename Cache<kt, dt>::value_compare value_compare;
  typedef typename Cache<kt, dt>::pointer pointer;
  typedef typename Cache<kt, dt>::reference reference;
  typedef typename Cache<kt, dt>::const_reference const_reference;
  typedef typename Cache<kt, dt>::size_type size_type;
  typedef typename Cache<kt, dt>::difference_type difference_type;
  typedef typename Cache<kt, dt>::iterator iterator;
  typedef typename Cache<kt, dt>::const_iterator const_iterator;
  typedef typename Cache<kt, dt>::reverse_iterator reverse_iterator;
  typedef typename Cache<kt, dt>::const_reverse_iterator const_reverse_iterator;

  ReferenceValueCache(): base_cache() {}
  ReferenceValueCache(const key_compare& comp): base_cache(comp) {}

  template <class InputIterator> ReferenceValueCache(InputIterator f, InputIterator l): base_cache(f, l) {}
  template <class InputIterator> ReferenceValueCache(InputIterator f, InputIterator l, const key_compare& comp): base_cache(f,l, comp) {}

  virtual ~ReferenceValueCache() { clear(); }


  void set_max_cached(int mc) {
    Synchronized(this);
    base_cache.set_max_cached(mc);
  }

  size_t max_cached() {
    Synchronized(this);
    return base_cache.max_cache();
  }

  iterator begin() {
    Synchronized(this);
    return base_cache.begin();
  }

  iterator end() {
    Synchronized(this);
    return base_cache.end();
  }

  const_iterator begin() const {
    return base_cache.begin();
  }

  const_iterator end() const {
    return base_cache.end();
  }

  reverse_iterator rbegin() {
    Syncrhonized(this);
    return base_cache.rbegin();
  }

  reverse_iterator rend() {
    Syncrhonized(this);
    return base_cache.rend();
  }

  const_reverse_iterator rbegin() const {
    return base_cache.rbegin();
  }

  const_reverse_iterator rend() const {
    return base_cache.rend();
  }

  size_type size() const {
    return base_cache.size();
  }

  size_type max_size() const {
    return base_cache.max_size();
  }

  bool empty() const {
    return base_cache.empty();
  }

  key_compare key_comp() const {
    return base_cache.key_comp();
  }

  value_compare value_comp() const {
    return base_cache.value_comp();
  }


  void swap(ReferenceValueCache& m2) {
    Synchronized(this);
    Synchronized(&m2);
    base_cache.swap(m2);
  }

  pair<iterator, bool> insert(const value_type& x) {
    Synchronized(this);
    return base_cache.insert(x);
  }

  iterator insert(iterator pos, const value_type& x) {
    Synchronized(this);
    return base_cache.insert(pos, x);
  }

  template <class InputIterator> void insert(InputIterator start, InputIterator end) {
    Synchronized(this);
    for (; start != end; ++start) {
      base_cache.insert(*start);
    }
  }

  void erase(iterator pos) {
    Synchronized(this);
    base_cache.erase(pos);
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
    iterator next;
    for (; first != last; first = next) {
      next = first;
      next++;
      erase(first);
    }
  }

  void clear() {
    Synchronized(this);
    erase(begin(), end());
  }

  iterator find(const key_type& k) {
    return base_cache.find(k);
  }

  const_iterator find(const key_type& k) const {
    return base_cache.find(k);
  }

  size_type count(const key_type& k) {
    Synchronized(this);
    return base_cache.count(k);
  }

  iterator lower_bound(const key_type& k) {
    Synchronized(this);
    return base_cache.lower_bound(k);
  }

  const_iterator lower_bound(const key_type& k) const {
    return base_cache.lower_bound(k);
  }

  iterator upper_bound(const key_type& k) {
    Synchronized(this);
    return base_cache.upper_bound(k);
  }

  const_iterator upper_bound(const key_type& k) const {
    return base_cache.upper_bound(k);
  }

  pair<iterator, iterator> equal_range(const key_type& k) {
    Syncrhonized(this);
    return base_cache.equal_range(k);
  }

  pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
    return base_cache.equal_range(k);
  }

  mapped_type& operator[](const key_type& k) {
    Synchronized(this);
    return base_cache[k];
  }

  bool operator==(const ReferenceValueCache& m2) const {
    return base_cache == m2.base_cache;
  }


  bool operator<(const ReferenceValueCache& m2)  const {
    return base_cache < m2.base_cache;
  }

  COMPARISON_OPERATORS(ReferenceValueCache);
};

}} //namespace

#endif // TEXTUS_BASE_REFERENCE_VALUE_CACHE_H_
