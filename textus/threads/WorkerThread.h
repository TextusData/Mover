/* WorkerThread.h -*- c++ -*-
 * Copyright (c) 2010-2012 Ross Biro
 *
 * Handle worker threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one worker thread per cpu is likely ideal.
 */

#include "textus/base/Thread.h"
#include "textus/event/EventQueue.h"
#include "textus/event/RoundRobinEventQueueScheduler.h"
#include "textus/threads/EventQueueThread.h"
#include "textus/base/Base.h"

#ifndef TEXTUS_THREADS_WORKER_THREAD_H_
#define TEXTUS_THREADS_WORKER_THREAD_H_

namespace textus { namespace threads {
using namespace textus::base;

class WorkerThread: public EventQueueThread {
private:

protected:

public:
  static textus::event::RoundRobinEventQueueScheduler *workerThreadScheduler();

  WorkerThread(): EventQueueThread(workerThreadScheduler()) { }
  virtual ~WorkerThread() { }

};

}} //namespace


#endif //TEXTUS_THREADS_WORKER_THREAD_H_
