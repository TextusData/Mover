/* BaseString.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Give std::string a Base.
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

#ifndef TEXTUS_BASE_BASE_STRING_H_
#define TEXTUS_BASE_BASE_STRING_H_

#include "textus/base/Base.h"

#include <string>

namespace textus { namespace base {

class BaseString: virtual public Base {
private:
  std::string data;
public:
  BaseString() {}
  virtual ~BaseString() {}
  operator std::string() const { 
    return data;
  }

  void append(std::string s) {
    data.append(s);
  }

  size_t length() {
    return data.length();
  }
};

}} //namespace


#endif // TEXTUS_BASE_BASE_STRING_H_
