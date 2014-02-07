/* CallBack.h -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Holds information for doing a callback.
 *
 */

#ifndef TEXTUS_BASE_CALL_BACK_H_
#define TEXTUS_BASE_CALL_BACK_H_

#include "textus/base/Base.h"
#include "textus/base/AutoDeref.h"

namespace textus { namespace base {

class CallBack:virtual  public Base {
public:
  typedef void (*CallBackFunc)(const CallBack *);

private:
  void *data_;
  CallBackFunc func_;

public:
  explicit CallBack(): func_(NULL) {}
  virtual ~CallBack() {}

  void setFunc(CallBackFunc f) {
    func_ = f;
  }

  CallBackFunc func() const {
    return func_;
  }

  void setData(void *b)  {
    data_ = b;
  }

  void *data() const { return data_; }
  
  virtual void do_it() {
    func_(this);
  }

};


}} // namespace

#endif // TEXTUS_BASE_CALL_BACK_H_
