/* CallBack.h -*- c++ -*-
 * Copyright (C) 2012 Ross Biro
 *
 * Holds information for doing a callback.
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
