/* CallBack.h -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Holds information for doing a callback on one or more other threads.
 *
 */

#ifndef TEXTUS_BASE_THREAD_CALL_BACK_H_
#define TEXTUS_BASE_THREAD_CALL_BACK_H_

#include "textus/base/CallBack.h"

namespace textus { namespace base {

class ThreadCallBack: public CallBack {
private:
  int doneCount_;

public:
  ThreadCallBack(): doneCount_(0) {}
  virtual ~ThreadCallBack() {}
  virtual void do_it();

  int threadsDone() {
    Synchronized(this);
    return doneCount_;
  }

};

}} // namespace

#endif // TEXTUS_BASE_THREAD_CALL_BACK_H_
