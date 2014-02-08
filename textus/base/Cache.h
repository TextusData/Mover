/* Cache.h -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * A template that represents a cache.
 *
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

#ifndef TEXTUS_BASE_CACHE_H_
#define TEXTUS_BASE_CACHE_H_

#include <map>
#include <list>

namespace textus { namespace base {
using namespace std;

// 20 is totally arbitrary.
template <class k1, class d1, size_t mcache=20> class Cache
{
private:
  size_t max_cached_;
  list<k1> lru;
  map<k1, d1> base_map;
  typedef typename map<k1, d1>::iterator map_iterator;

public:
  typedef typename map<k1, d1>::key_type key_type;
  typedef typename map<k1, d1>::mapped_type mapped_type;
  typedef typename map<k1, d1>::value_type value_type;
  typedef typename map<k1, d1>::key_compare key_compare;
  typedef typename map<k1, d1>::value_compare value_compare;
  typedef typename map<k1, d1>::pointer pointer;
  typedef typename map<k1, d1>::reference reference;
  typedef typename map<k1, d1>::const_reference const_reference;
  typedef typename map<k1, d1>::size_type size_type;
  typedef typename map<k1, d1>::difference_type difference_type;
  typedef typename map<k1, d1>::const_iterator const_iterator;
  typedef typename map<k1, d1>::reverse_iterator reverse_iterator;
  typedef typename map<k1, d1>::const_reverse_iterator const_reverse_iterator;

  friend class iterator;

  class iterator: public map_iterator {
  private:
    Cache *parent;

  public:
    explicit iterator(Cache *p, map_iterator i): map_iterator(i), parent(p) {
      if (i != p->base_map.end())
	p->touch(i->first);
    }

    explicit iterator(): parent(NULL) {}

    ~iterator() {}

    void touch();

    iterator& operator++() { touch(); map_iterator::operator ++(); return *this; }
    iterator operator++(int) {
      touch();
      iterator __tmp = *this;
      map_iterator::operator ++();
      return __tmp;
    }
    
    iterator& operator--() { touch(); map_iterator::operator --(); return *this; }
    iterator operator--(int) {
      touch();
      iterator __tmp = *this;
      map_iterator::operator --();
      return __tmp;
    }
  };

  Cache():max_cached_(mcache) {}
  virtual ~Cache() {}

  void set_max_cached(int mc) {
    max_cached_ = mc;
  }

  size_t max_cached() {
    return max_cached_;
  }

  void touch(key_type k) {
    lru.remove(k);
    lru.push_front(k);
  };

  void prune(size_t count) {
    size_t s_size = base_map.size();
    while (!base_map.empty() && base_map.size() > s_size - count) {
      assert(!lru.empty());
      key_type k = lru.back();
      lru.pop_back();
      base_map.erase(k);
    }
  }

  iterator begin() {
    return iterator(this, base_map.begin());
  }

  iterator end() {
    return iterator(this, base_map.end());
  }

  const_iterator begin() const {
    return base_map.begin();
  }

  const_iterator end() const {
    return base_map.end();
  }

  reverse_iterator rbegin() {
    return base_map.rbegin();
  }

  reverse_iterator rend() {
    return base_map.rend();
  }

  const_reverse_iterator rbegin() const {
    return base_map.rbegin();
  }

  const_reverse_iterator rend() const {
    return base_map.rend();
  }

  size_type size() const {
    return base_map.size();
  }

  size_type max_size() const {
    return base_map.max_size();
  }

  bool empty() const {
    return base_map.empty();
  }

  key_compare key_comp() const {
    return base_map.key_comp();
  }

  value_compare value_comp() const {
    return base_map.value_comp();
  }

  void swap(Cache& m2) {
    base_map.swap(m2);
    size_t m = max_cached_;
    max_cached_=m2.max_cached_;
    m2.max_cached_ = m;
  }

  pair<iterator, bool> insert(const value_type& x) {
    if (base_map.size() >= max_cached_) {
      prune(1);
    }
    touch(x.first);
    return base_map.insert(x);
  }

  iterator insert(iterator pos, const value_type& x) {
    if (base_map.size() >= max_cached_) {
      prune(1);
    }
    touch(x.first);
    return base_map.insert(pos, x);
  }

  template <class InputIterator> void insert(InputIterator start, InputIterator end) {
    for (; start != end; ++start) {
      if (base_map.size() >= max_cached_) {
	prune(1);
      }
      touch(*start.first);
      base_map.insert(*start);
    }
  }

  void erase(iterator pos) {
    key_type k = pos->first;
    base_map.erase(pos);
    touch(k);
  }

  size_type erase(const key_type& k) {
    size_type c=0;
    for (iterator i = find(k); i != end(); i = find(k)) {
      erase(i);
      ++c;
    }
    return c;
  }

  void erase(iterator first, iterator last) {
    for (; first != last; ++first) {
      erase(first);
    }
  }

  void clear() {
    erase(begin(), end());
  }

  iterator find(const key_type& k) {
    return iterator(this, base_map.find(k));
  }

  const_iterator find(const key_type& k) const {
    return base_map.find(k);
  }

  size_type count(const key_type& k) {
    touch(k);
    return base_map.count(k);
  }

  iterator lower_bound(const key_type& k) {
    return iterator(this, base_map.lower_bound(k));
  }

  const_iterator lower_bound(const key_type& k) const {
    return base_map.lower_bound(k);
  }

  iterator upper_bound(const key_type& k) {
    return iterator(this, base_map.upper_bound(k));
  }

  const_iterator upper_bound(const key_type& k) const {
    return base_map.upper_bound(k);
  }

  pair<iterator, iterator> equal_range(const key_type& k) {
    touch(k);
    return base_map.equal_range(k);
  }

  pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
    return base_map.equal_range(k);
  }

  mapped_type& operator[](const key_type& k) {
    touch(k);
    if (base_map.size() >= max_cached_ && base_map.find(k) == end()) {
      prune(1);
    }
    return base_map[k];
  }

  bool operator==(const Cache& m2) const {
    return base_map == m2.base_map;
  }


  bool operator<(const Cache& m2)  const {
    return base_map < m2.base_map;
  }

  COMPARISON_OPERATORS(Cache);

};

template<class a, class b, size_t c> inline void Cache<a,b, c>::iterator::touch() {
  parent->touch(this->operator *().first);
}



}} // namespace

#endif // TEXTUS_BASE_CACHE_H_
