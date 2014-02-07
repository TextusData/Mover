/* BaseString.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Give std::string a Base.
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
