/* WaitingTask.h -*- c++ -*-
 * Copyright (c) Ross Biro 2012
 *
 * Encapsulates the information for things a thread should
 * do before going to sleep.
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
