/* WorkerThread.cc -*- c++ -*-
 * Copyright (c) 2010-2012, 2014 Ross Biro
 *
 * Handle worker threads.  Mostly these
 * Threads wait on event queues, and then
 * process the events.  If events are all
 * able to be processed in a non-blocking manner,
 * one worker thread per cpu is likely ideal.
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

Thread *WorkerThread::getFirst() {
  // Don't need to worry about derefing this.
  // the copy in the thread array takes care of it.
  static Thread *ft = NULL;
  static Base lock;
  Synchronized(&lock);
  if (ft != NULL) {
    return ft;
  }
  ft = new WorkerThread();
  return ft;
}

DEFINE_INIT_FUNCTION(workerThreadStartup, AUXILLARY_THREAD_INIT_PRIORITY + 1) {
  Thread *first_thread;

  first_thread  = WorkerThread::getFirst();

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

DEFINE_SHUTDOWN_FUNCTION(workerThreadShutdown, 
			 AUXILLARY_THREAD_SHUTDOWN_PRIORITY + 1) {
  WorkerThread::workerThreadScheduler()->shutdown();
  for (int i = 0; i < worker_thread_count; i++) {
    worker_thread_array[i]->join();
    worker_thread_array[i]->deref();
  }
  free (worker_thread_array);
}

}} //namespace
