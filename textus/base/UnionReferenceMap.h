/* UnionReferenceMap.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
 *
 */

#include "textus/base/Base.h"

#ifndef TEXUS_BASE_UNIONREFERENCEMAP_H_
#define TEXUS_BASE_UNIONREFERENCEMAP_H_

namespace textus { namespace base {

template <typename M, typename V> class UnionReferenceMap: virtual public Base {
private:
  typedef ReferenceList<M *> maps_type;
  typedef typename ReferenceList<M>::iterator maps_iterator;

  maps_type maps;

protected:
public:
  explicit UnionReferenceMap() {}
  virtual ~UnionReferenceMap() {}
  void addMap(M *m) {
    maps.push_back(m);
  }

  size_t count(typename M::key_type k) {
    size_t c = 0;
    foreach(i, maps) {
      c = (*i)->count(k);
    }
    return c;
  }

  bool contains(typename M::key_type k) {
    foreach(i, maps) {
      if ((*i)->count(k) > 0) {
	return true;
      }
    }
    return false;
  }

  V at(typename M::key_type k) {
    foreach(i, maps) {
      if ((*i)->count(k) > 0) {
	M *m = *i;
	return m->operator[](k);
      }
    }
    return maps.front()->operator[](k);
  }

  

};

}} // namespace


#endif //TEXUS_BASE_UNIONREFERENCEMAP_H_
