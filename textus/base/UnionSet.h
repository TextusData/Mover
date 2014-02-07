/* UnionSet.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * A set that is a union of other sets.
 * You remove sets from the current one
 * by using reference counts.
 * 
 * That way, we always have quick
 * access to the members.
 */


#ifndef TEXTUS_BASE_UNION_SET_H_
#define TEXTUS_BASE_UNION_SET_H_

#include <map>
#include <set>

namespace textus { namespace base {

template <typename T> class UnionSet {
private:
  map<T, int> base;
  set<T> keyset;

public:
  explicit UnionSet() {}
  ~UnionSet() {}
  void addSet(const set<T> &s) {
    for (typename set<T>::const_iterator i = s.begin(); i != s.end(); ++i) {
      if (base.count(*i) > 0) {
	++base[*i];
      } else {
	base[*i] = 1;
	keyset.insert(*i);
      }
    }
  }

  void removeSet(const set<T> &s) {
    for (typename set<T>::const_iterator i = s.begin(); i != s.end(); ++i) {
      if (base.count(*i) > 0) {
	if (--base[*i] == 0) {
	  base.erase(*i);
	  keyset.erase(*i);
	}
      }
    }
  }

  void count(T key) {
    if (base.has_key(key)) {
      return base[key];
    }
    return 0;
  }

  const set<string> &getKeyset() const {
    return keyset;
  }

  operator set<T> const () {
    if (keyset.size() > 0) {
      return keyset;
    }

    for(typename map<T, int>::const_iterator i = base.begin(); i != base.end(); ++i) {
      keyset.insert(i->first);
    }
  }
  
};

}} //namespace

#endif // TEXTUS_BASE_UNION_SET_H_
