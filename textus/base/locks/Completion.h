/* Completion.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Block until an event occurs.
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

#ifndef TEXTUS_BASE_LOCKS_COMPLETION_H_
#define TEXTUS_BASE_LOCKS_COMPLETION_H_

#include "textus/base/Base.h"
#include "textus/base/logic/Bool.h"

namespace textus { namespace base { namespace locks {

class Completion: virtual public Base {
private:
  textus::base::logic::Bool done;

public:
  Completion():done(false) {}
  virtual ~Completion() {}

  void complete() {
    Synchronized(this);
    done = true;
    signal();
  }

  void wait() {
    Synchronized(this);
    while(!done) {
      Base::wait();
    }
  }

  void reset()  {
    Synchronized(this);
    done = false;
  }
  
};

}}} //namespace

#endif // TEXTUS_BASE_LOCKS_COMPLETION_H_

