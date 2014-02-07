/* Base.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 * 
 * A class that represents some kind of error.
 */

#ifndef TEXTUS_BASE_ERROR_H_
#define TEXTUS_BASE_ERROR_H_

#include <errno.h>

#include "textus/base/Base.h"

namespace textus { namespace base {

class Error: public Base {
private:
  int err;

public:
  Error(int e=-1) {
    if (e == -1) {
      e = errno;
    }
    err = e;
  }

  virtual ~Error() {}
  virtual std::ostream &doPrint(std::ostream &o) const {
    if (err > 0) {
      o << strerror(err);
    } else {
      o << "No Error";
    }
    return o;
  }
};

}} //namespace

#endif //TEXTUS_BASE_ERROR_H_
