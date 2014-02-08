/* STLExtensions.h -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
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
