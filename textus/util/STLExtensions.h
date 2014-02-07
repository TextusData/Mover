/* STLExtensions.h -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
 *
 */

#ifndef TEXTUS_UTIL_STLEXTENSIONS_H_
#define TEXTUS_UTIL_STLEXTENSIONS_H_

#include <list>

namespace textus { namespace util {

template <typename T> class listext  {
public:
  static bool contains(const std::list<T> &l, const T &t) {
    for (typename std::list<T>::const_iterator i = l.begin();
	 i != l.end(); ++i) {
      if (*i == t) {
	return true;
      }
    }
    return false;
  }
};

}} //namespace


#endif //TEXTUS_UTIL_STLEXTENSIONS_H_
