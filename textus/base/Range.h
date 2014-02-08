/* Range.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Represents a range of numbers.
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

#ifndef TEXTUS_BASE_RANGE_H_
#define TEXTUS_BASE_RANGE_H_

#include "textus/base/init/Init.h"

namespace textus { namespace base {


template <typename T, int (*F)(const char *) > class Range {
private:
  T min_;
  T max_;
public:
  explicit Range() {}
  explicit Range(T mi, T ma): min_(mi), max_(ma) {}
  ~Range() {}

  T min() const {
    return min_;
  }

  T max() const {
    return max_;
  }

  void setmin(T m) {
    min_ = m;
  }

  void setmax(T m) {
    max_ = m;
  }

  T length() const {
    return max_ - min_;
  }

  bool contains(T t) const {
    return (t >= min_ && t < max_);
  }

  Range *operator= (const std::string &s) {
    converter_Range(s, this);
  }

// syntax is a-b
  static int converter_Range(const string &s, void *v) {
    Range *rp = static_cast < Range * > (v);
    size_t d = s.find('-');
    if (d == string::npos) {
      rp->setmin(F(s.c_str()));
    } else {
      if (d != 0) {
	rp->setmin(F(s.substr(0, d-1).c_str()));
      }
      if (d < s.length() - 1) {
	rp->setmax(F(s.substr(d+1).c_str()));
      }
    }
    return 0;
  }

};


}} //namespace

#define DECLARE_RANGE_ARG(T, atot, name) DECLARE_CUSTOM_ARG(Range<T, atot>, name)
#define DEFINE_RANGE_ARG(T, atot, name, def, argument, description)	\
  DEFINE_CUSTOM_ARG((Range<T, atot>) , name, def, (textus::base::Range<T, atot>::converter_Range), argument, description)

#define DECLARE_INT_RANGE_ARG(name) DECLARE_RANGE_ARG(int, atoi, name)
#define DEFINE_INT_RANGE_ARG(name, def, argument, description) \
  DEFINE_RANGE_ARG(int, atoi, name, def, argument, description)

#endif // TEXTUS_BASE_RANGE_H_
