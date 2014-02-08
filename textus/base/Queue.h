/* Queue.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Template for implementing a local quque.
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


#ifndef TEXTUS_BASE_QUEUE_H_
#define TEXTUS_BASE_QUEUE_H_

#include "textus/base/ReferenceList.h"

namespace textus { namespace base {

template <class T> class Queue: public virtual Base {
 private:
  ReferenceList<T> list;

 public:
  explicit Queue() {}
  virtual ~Queue() {}

  void add(T t) {
    Synchronized(this);
    list.push_back(t);
  }

  T get(void) {
    Synchronized(this);
    if (list.empty()) {
      return 0;
    }
    T t=list.front();
    list.pop_front();
    return t;
  }

  size_t size() const {
    return list.size();
  }

  void clear() {
    Synchronized(this);
    list.clear();
  }

  void erase(T t) {
    Synchronized(this);
    list.erase(t);
  }

};

}} //namespacd

#endif //TEXTUS_BASE_QUEUE_H_
