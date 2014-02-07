/* Identity.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Misc macros and related.
 */


#ifndef TEXTUS_BASE_UTILITY_H
#define TEXTUS_BASE_UTILITY_H

#include <sstream>
#include <list>
#include <string>
#include <iostream>

namespace textus { namespace base {

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define COMPARISON_OPERATORS2(type1, type2)				       \
  bool operator <= (type2 rhs) const { return (*this == rhs || *this < rhs); } \
  bool operator  > (type2 rhs) const { return !(*this <= rhs); }		\
  bool operator >= (type2 rhs) const { return !(*this < rhs); }			\
  bool operator != (type2 rhs) const { return !(*this == rhs); }

#define COMPARISON_OPERATORS(type) \
  bool operator <= (const type &rhs) const { return (*this == rhs || *this < rhs); }	\
  bool operator  > (const type &rhs) const { return !(*this <= rhs); }			\
  bool operator >= (const type &rhs) const { return !(*this < rhs); }			\
  bool operator != (const type &rhs) const { return !(*this == rhs); }

template <typename T>
static inline std::string to_string (const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

template <typename T>
static inline std::string join(const T &s, std::string sep=", ") {
  std::string r;
  for (typename T::iterator i = s.begin(); i != s.end(); ++i) {
    std::string sp = to_string<typename T::value_type>(*i);
    if (r.length() == 0) {
      r = sp;
    } else {
      r = r + sep + sp;
    }
  }
  return r;
}

static inline std::list<std::string> split(const std::string &s, char sep=' ') {
  std::stringstream ss(s);
  std::list<std::string> l;
  std::string item;
  while (std::getline(ss, item, sep)) {
    l.push_back(item);
  }

  return l;
}

}} //namespace

#endif // TEXTUS_BASE_UTILITY_H
