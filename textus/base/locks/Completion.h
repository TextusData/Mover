/* Completion.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Block until an event occurs.
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

