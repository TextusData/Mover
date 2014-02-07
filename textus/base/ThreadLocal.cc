/* ThreadLocal.cc -*- C++ -*-
 * Copyright (C) 2009-2013 Ross Biro
 *
 * A thread local template.  Creates a
 * thrad local variable of a given type.
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
