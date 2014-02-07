/* WorkerThread.cc -*- c++ -*-
 * Copyright (c) 2010-2012 Ross Biro
 *
 * Handle worker threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one worker thread per cpu is likely ideal.
 */

#include "textus/threads/WorkerThread.h"
#include "textus/system/SysInfo.h"

namespace textus { namespace threads {

using namespace textus::base;

textus::event::RoundRobinEventQueueScheduler *WorkerThread::workerThreadScheduler() {
  static textus::event::RoundRobinEventQueueScheduler rres;
  return &rres;
}

DEFINE_INT_ARG(worker_thread_count, -1, "worker_thread_count", "Number of worker threads to fire up.  Defaults to number of cpus.");
static Thread **worker_thread_array;

DEFINE_INIT_FUNCTION(workerThreadStartup, AUXILLARY_THREAD_INIT_PRIORITY + 1) {
  Thread *first_thread;
  if (worker_thread_count != 0) {
    // Fire up one first so that getNUMCPUs will work.
    first_thread = new WorkerThread();
  }

  if (worker_thread_count < 0) {
    worker_thread_count = textus::system::SysInfo::systemInformation()->getNumCPUs();
  }

  worker_thread_array = static_cast<Thread **>(malloc(sizeof(*worker_thread_array)*worker_thread_count));
  worker_thread_array[0] = first_thread;

  // start at 1 since we already fired one up.
  for (int i=1; i < worker_thread_count; i++) {
    // dereffed when the thread exits.
    worker_thread_array[i] = new WorkerThread();
  }

  return 0;
}

DEFINE_SHUTDOWN_FUNCTION(workerThreadShutdown, AUXILLARY_THREAD_INIT_PRIORITY + 1) {
  WorkerThread::workerThreadScheduler()->shutdown();
  for (int i = 0; i < worker_thread_count; i++) {
    worker_thread_array[i]->join();
    worker_thread_array[i]->deref();
  }
  free (worker_thread_array);
}

}} //namespace