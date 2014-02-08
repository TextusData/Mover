/* Base.h -*- C++ -*-
 * Copyright (C) 2010 Ross Biro
 * 
 * A class that represents some kind of error.
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
