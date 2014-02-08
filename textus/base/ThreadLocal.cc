/* ThreadLocal.cc -*- C++ -*-
 * Copyright (C) 2009-2013 Ross Biro
 *
 * A thread local template.  Creates a
 * thrad local variable of a given type.
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
#include <pthread.h>
#include "textus/logging/Log.h"
#include "textus/base/ThreadLocal.h"

namespace textus { namespace base {

ReferenceList<ThreadLocalBase *> &ThreadLocalBase::locals() {
  static ReferenceList<ThreadLocalBase *> static_local;
  return static_local;
};


ThreadLocalBase::ThreadLocalBase() {
  Synchronized(&locals());
  locals().push_back(this);
}

ThreadLocalBase::~ThreadLocalBase() {
  Synchronized(&locals());
  locals().erase(this);
}

void ThreadLocalBase::threadExit() {
  Synchronized(&locals());
  for (ReferenceList<ThreadLocalBase *>::iterator i = locals().begin();
       i != locals().end();
       ++i) {
    (*i)->cleanup();
  }
}

void ThreadLocalBase::static_cleanup(void *v) 
{
  ThreadLocalBase *tl = static_cast<ThreadLocalBase *>(v);
  tl->cleanup();
}

void ThreadLocalBase::updateValueCallBack(const CallBack *cb) {
  const ThreadLocalCallBack *tlcb = dynamic_cast<const ThreadLocalCallBack *>(cb);
  if (tlcb == NULL) {
    LOG(ERROR) << "updateValueCallBack called with non ThreadLocalCallBack.\n";
    die();
  }
  ThreadLocalBase *tlb = tlcb->getThreadLocal();
  tlb->setWithVptr(cb->data());
}

}} // namespace
