/* Thread.cc -*- c++ -*-
 * Copyright (c) 2009-2014 Ross Biro
 *
 * Encapsulate the idea of a thread.
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

#include <signal.h>

#include "textus/base/Thread.h"
#include "textus/base/ThreadCallBack.h"


namespace textus { namespace base {

Mutex single_threaded;
Base thread_stat_lock;

#ifdef DEBUG_LOCKS
std::list<int> Thread::static_lock_levels;
#endif

int Thread::ThreadsCreated = 0;
int Thread::ThreadsDeleted = 0;
int Thread::ThreadsJoined = 0;
int Thread::ThreadsExited = 0;
WeakReferenceList<Thread *> Thread::threadList;

pthread_key_t Thread::thread_class_key;
bool Thread::initialized = false;

void Thread::threadDerefVoid(void *vt) 
{
  Thread *t = static_cast<Thread *>(vt);
  assert(0); // this should never be called.
  if (t == NULL) {
    LOG(ERROR) << "threadDerefVoid called on NULL thread, while vt = " << vt << "\n";
    die();
  }
  threadDeref(t);
}

void Thread::threadDeref(Thread *t) {
  threadList.erase(t);
  pthread_setspecific(Thread::thread_class_key, NULL);
  if (t) {
    t->is_done = true;
    t->deref();
  } else {
    LOG(WARNING) << "threadDeref Called on NULL.  This means there was a thread with out a Thread class.\n";
  }
}

// Called before the thread waits.  Needs to
// check the thread for something to do
// and return quickly if there is nothing.
bool threadWaiting(Mutex *m) {
  Thread *t = Thread::self();
  if (!t) {
    return false;
  }
  return t->waitingTasks(m);
}

static ThreadID startupThread;

DEFINE_SHUTDOWN_FUNCTION(threadShutdown, THREAD_SHUTDOWN_PRIORITY)
{
  single_threaded.lock();
  // We need to kill the startup thread, which better be
  // us.
  LOG(DEBUG) << "threadShutdown:\n";
  if (startupThread != ThreadID::self()) {
    LOG(ERROR) << "shutdown thread not the same as startup thread.\n";
  }
  Thread *t = Thread::self();
  Thread::threadDeref(t);
  pthread_key_delete(Thread::thread_class_key);
  single_threaded.unlock(); // can't keep a lock over shutdown.
  LOG(DEBUG) << "Exiting threadShutdown\n";

}

static void sigusr1Handler(int) {
  AUTODEREF(Thread *, t);
  t = Thread::self();
  t->ref();
  t->handleSignal();
}

DEFINE_INIT_FUNCTION(threadInit, THREAD_INIT_PRIORITY)
{
  int ret = pthread_key_create(&Thread::thread_class_key, Thread::threadDerefVoid);
  if (ret )
    return ret;

  /* Now we need to create the Thread class for the current thread. 
     This is not a memory leak.  The thread will be referenced via
     the thread local storage.  We will need to clean it up though on
     shutdown. */

  new Thread();

  startupThread = ThreadID::self();

  signal(SIGUSR1, sigusr1Handler);
  mb();
  Thread::initialized = true;
  return 0;
}

void *Thread::threadStartup(void *arg)
{
  AUTODEREF(Thread *, th);
  th = static_cast<Thread *>(arg);
  // Wait until we get the thread, then release it immediately.
  // This way we know that the Thread constructor has completed.
  {
    Synchronized(th);
  }

  pthread_setspecific(thread_class_key, th);

  Base *ret =  th->start_up(th->argument);

  Synchronized(th);

  pthread_setspecific(thread_class_key, NULL);
  th->argument = NULL;

  {
    Synchronized(&thread_stat_lock);
    threadList.erase(th);
  } // now that we are off the list, others should not find us.

  th->cleanup();

  // Don't ref because we got the ref from
  // function that just returned.
  if (ret && th->return_value != ret) {
    LOG(WARNING) << "ThreadStartup replacing return_value " << th->return_value << " for Thread " << th << " with " << ret << ".\n";
    th->return_value = ret;
  }

  return ret;
}

// handles sig_user1.  Disptaches it acording to the
// settings of signal_type and signal_message.
// This should be a rare call.  It's only
// used to wake up certain queue threads (signal_type = -1)
// or for emergency situations.
void Thread::handleSignal()
{
  int type;
  AUTODEREF(Base *, message);
  {
    message = signal_message;
    type = signal_type;
    signal_message = NULL;
    signal_type = -1;
  }

  switch (type) {
  case SigDump:
    LOG(ERROR) << "Thread: received dump signal:\n";
    LOG_CALL_STACK(ERROR);
    exit(message);
    break;

  default:
    // common do nothing.  The side effect of causing
    // a eintr from certain sys calls is what we wanted.
    break;
  }
}

void Thread::doAllThreadsWait(class ThreadCallBack *cb) {
  int thread_count = 0;
  {
    Synchronized(&thread_stat_lock);
    for (WeakReferenceList<Thread *>::iterator i = threadList.begin();
	 i != threadList.end(); ++i) {
      Thread *th = *i;
      if (th->done()) {
	continue;
      }
      {
	Synchronized(&th->waitingTaskList);
	th->waitingTaskList.push_back(cb);
      }
      if (th->getDoAll()) {
	thread_count ++;
      }
      th->signal(); // only way we know we can get a message to all threads quickly.
    }
  } // Drop the lock.

  // Make sure we do it ourselves, right now.  The wait below should take care of it
  // as well, but best to be sure.
  while (Thread::self()->waitingTasks(NULL));
  Synchronized(cb);

  while (thread_count > cb->threadsDone()) {
    cb->wait();
  }
}

void Thread::cleanup() {
  // the is_done should prevent anything from
  // being added to the thread list.  We 
  {
    Synchronized(this);
    {
      Synchronized(&thread_stat_lock);
      threadList.erase(this);
    } // now that we are off the list, others should not find us.
    is_done = true;
  }
  while (waitingTasks(NULL));

  ThreadLocalBase::threadExit();

  Synchronized(&thread_stat_lock);

  ThreadsExited++;
    

}

}} //namespace
