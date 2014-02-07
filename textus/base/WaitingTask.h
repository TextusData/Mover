/* WaitingTask.h -*- c++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * Encapsulates the information for things a thread should
 * do before going to sleep.
 */

#ifndef TEXTUS_BASE_WAITING_CLASS_H_
#define TEXTUS_BASE_WAITING_CLASS_H_

namespace textus { namespace base {

class WaitingTask: virtual public Base {
private:
  AutoDeref<CallBack> cb;
protected:
public:
  explicit WaitingTask() {}
  virtual ~WaitingTask() {}

  CallBack *callback() { return cb; }
  void setCallBack(CallBack *c) { cb = c; }
  void doIt() { cb->doIt(); }
};

}} // namespace
#endif //TEXTUS_BASE_WAITING_CLASS_H_
