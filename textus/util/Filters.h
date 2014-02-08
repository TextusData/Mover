/* Filters.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
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

#ifndef TEXTUS_UTIL_FILTERS_H_
#define TEXTUS_UTIL_FILTERS_H_

#include <utility>

namespace textus { namespace util {

template <typename T1, T1 v1, typename T2, T2 v2>
bool pair_equals(const std::pair<T1, T2> &p) {
  return (p.first == v1 && p.second == v2);
}

}}

#endif //TEXTUS_UTIL_FILTERS_H_
