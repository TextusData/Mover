/* Filters.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
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
