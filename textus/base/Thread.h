/* Thread.h -*- c++ -*-
 * Copyright (c) 2009-2013 Ross Biro
 *
 * Encapsulate the idea of a thread.
 */

#ifndef TEXTUS_BASE_THREAD_H_
#define TEXTUS_BASE_THREAD_H_

#include <stdio.h>

#include <signal.h>
#include <list>

#include "textus/base/Debug.h"
#include "textus/base/Base.h"
#include "textus/base/ThreadCallBack.h"
#include "textus/base/logic/Bool.h"
#include "textus/base/ThreadID.h"
#include "textus/base/init/Init.h"
#include "textus/base/locks/Lock.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/WeakReferenceList.h"
#include "textus/base/ThreadLocal.h"

namespace textus { namespace base {

template <class T> class ReferenceList;
template <class T> class WeakReferenceList;

extern Mutex single_threaded;
extern Base thread_stat_lock;

class ThreadCallBack;

class Thread: virtual public Base {
public:
  typedef Base *(*start_routine)(Base *);
  enum {
    SigDump = 1,
  };

private:
  static WeakReferenceList<Thread *> threadList; // use the stats lock for this as well.
  static int ThreadsCreated;
  static int ThreadsDeleted;
  static int ThreadsJoined;
  static int ThreadsExited;

  ThreadID id;
  ReferenceList<ThreadCallBack *> waitingTaskList;
  AutoDeref<Base> return_value;
  logic::Bool is_done;
  int signal_type;
  AutoDeref<Base> signal_message;
  bool no_do_all;

#ifdef DEBUG_LOCKS
  static std::list<int> static_lock_levels; //used before first thread class exists.
  std::list<int> lock_levels;
#endif

  static pthread_key_t thread_class_key;
  static bool initialized;
  static void *threadStartup(void *);
  static void threadDerefVoid(void *);
  static void threadDeref(Thread *);

  void cleanup();

  friend DECLARE_INIT_FUNCTION(threadInit);
  friend DECLARE_SHUTDOWN_FUNCTION(threadShutdown);

  // Private initializer only used to attach to existing threads.
  Thread(): id(), is_done(false), signal_type(-1), no_do_all(false)
  {
    id.id = ThreadID::self().id;
    ThreadsCreated++;
    pthread_setspecific(thread_class_key, this);
    threadList.push_back(this);
#ifdef DEBUG_LOCKS
    lock_levels=static_lock_levels;
    static_lock_levels.clear();
#endif
  }

protected:
  start_routine start_up;
  AutoDeref<Base> argument;

  int weakDelete() {
    LOG(ERROR) << "Weak Delete called on thread. thread didn't exit properly.\n";
    die();
    return Base::weakDelete();
  }

public:

  static void doAllThreadsWait(ThreadCallBack *cb);

  static Thread *self() {
    if (!initialized) {
      return NULL;
    }
    Thread *th = reinterpret_cast<Thread *>(pthread_getspecific(thread_class_key));
    return th;
  }

  Thread (start_routine start, Base *arg): id(), is_done(false), signal_type(-1), no_do_all(false)
  {
    pthread_t tid;
    start_up = start;
    argument = arg;
      
    // Grab a lock so we can control the start order.
    // The thread start routine will not be called
    // until we return.
    Synchronized(this);
    if (!start_up) {
      block();
    }

    ref(); // this extra ref is handed to the created thread.

    pthread_create(&tid, NULL, threadStartup, this);
    id.id = tid;
    Synchronized(&thread_stat_lock);
    ThreadsCreated++;
    LOG(DEBUG) << " Created thread( " << ThreadsCreated << ") id: " << (void *)tid << "\n";
    threadList.push_back(this);
    if (ThreadsCreated == 1) {
      single_threaded.lock();
    }
  }

  bool getDoAll() {
    return !no_do_all;
  }

  void setDoAll(bool da) {
    no_do_all = !da;
  }

  int yield() {
    return pthread_yield();
  }

  virtual ~Thread()
  {
    if (!is_done) {
      LOG(ERROR) << " Thread destructor for " << *this << " called while thread not done.\n";
      LOG_CALL_STACK(ERROR);
      die();
    }
    is_done = false;
    Synchronized(&thread_stat_lock);
    ThreadsDeleted++;
    if (ThreadsDeleted == ThreadsCreated) {
      single_threaded.unlock();
    }
  }

  bool waitingTasks(Mutex *m) {
    // racy check w/o the lock.
    if (waitingTaskList.empty()) {
      return false;
    }

    Synchronized(&waitingTaskList);

    bool didSomething = false;
    // check again now that we know it's stable
    while (!waitingTaskList.empty()) {
      if (!didSomething) {
	didSomething = true;
	if (m) {
	  m->unlock();
	}
      }
      AUTODEREF(ThreadCallBack *, wt);
      wt = waitingTaskList.front();
      wt->ref();
      waitingTaskList.pop_front();
      wt->do_it();
    }
    if (didSomething && m) {
      m->lock();
    }
    return didSomething;
  }

  virtual std::ostream& doPrint(std::ostream &o) const
  {
    // All we know about a base is that it's a base and it has an address.
    o << "Thread-" << id;
    return o;
  }

#ifdef DEBUG_LOCKS
  static int staticGetLockLevel() {
    int ret;
    if (static_lock_levels.empty()) {
      ret = INT_MAX;
    } else {
      ret = static_lock_levels.front();
    }
    LOG(DEBUG) << "sll = " << ret << "\n";
    return ret;
  }

  int getLockLevel() const {
    int ret;
    if (lock_levels.empty()) {
      ret = INT_MAX;
    } else {
      ret = lock_levels.front();
    }
    LOG(DEBUG) << "ll = " << ret << "\n";
    return ret;
  }

  static void staticPushLockLevel(int ll) {
    LOG(DEBUG) << "pushing sll" << ll << "\n";
    static_lock_levels.push_front(ll);
  }

  void pushLockLevel(int ll) {
    LOG(DEBUG) << "pushing ll" << ll << "\n";
    lock_levels.push_front(ll);
  }

  void popLockLevel() {
    assert(!lock_levels.empty());
    int ret = getLockLevel();
    lock_levels.pop_front();
    LOG(DEBUG) << "popping ll " << ret << " "
	       << "cll now " << getLockLevel() << " \n";
  }

  static void staticPopLockLevel() {
    assert(!static_lock_levels.empty());
    int ret = staticGetLockLevel();
    static_lock_levels.pop_front();
    LOG(DEBUG) << "popping sll" << ret << " "
	       << "scll now " << staticGetLockLevel() << " \n";
  }

#endif


  void exit(Base *retval)
  {
    {
      Synchronized(this);
      // Make sure it's the right thread calling.
      if (id != ThreadID::self()) {
	LOG(ERROR) << "Thread Exit called from " <<  ThreadID::self()  <<  " for thread " << this <<  ".\n";
	LOG_CALL_STACK(ERROR);
	die();
      }

      // Don't need the argument anymore.
      if (argument)
	argument->deref();

      argument = NULL;

      return_value = retval;
      if (return_value) {
	return_value->ref();
      }

      is_done = true;
      mb();
    } // drop the thread lock.  
  
    cleanup();
    pthread_exit(retval);
    /* Not Reached */
  }

  int join()
  {
    void *jr;
    int retval = pthread_join(id.id, &jr);
    if (retval) {
      return retval;
    }

    Base *join_return = static_cast<Base *>(jr);

    {
      Synchronized(&thread_stat_lock);
      ThreadsJoined++;
    }

    // Shouldn't happen, but could if the thread
    // mistakenly calls pthread_exit itself.
    if (join_return && join_return != return_value) {
      Synchronized(this);
      LOG(WARNING) << "Thread Join replacing return_value " << return_value << " with " << join_return;
      if (return_value) {
	return_value->deref();
      }
      return_value = join_return;
      return_value->ref();
    }
    return 0;
  }

  bool done() {
    Synchronized(this);
    return is_done;
  }

  // ref's before returning so
  // the caller must deref.
  Base *returnValue() {
    Synchronized(this);
    if (return_value)
      return_value->ref();
    return return_value;
  }

  // used to send an immediate message
  // to another thread.  Can't buffer
  // messages, so only use in dire
  // situations, like in die to 
  // grab information.
  int signal(int type = -1, Base *message = NULL)
  {
    Synchronized(this);
    if (type != -1) {
      signal_type = type;
      signal_message = message;
    }
    return pthread_kill (id.id, SIGUSR1);
  }

  void handleSignal();
};

bool threadWaiting( Mutex *m);


}} // namespace


// startup and shutdown priority of the thread subsystem.  Must
// startup before any new threads are created and shutdown after they
// are destroyed.
#define THREAD_INIT_PRIORITY 100
#define THREAD_SHUTDOWN_PRIORITY 100

// Auxiallary thread startup and shutodwn priority.  Must be larger than
// the thread startup and shutdown priority.
#define AUXILLARY_THREAD_INIT_PRIORITY 1000
#define AUXILLARY_THREAD_SHUTDOWN_PRIORITY 1000

#define INITTHREAD(startup, arg)					\
  DEFINE_INIT_FUNCTION(UNIQUE_IDENTIFIER(InitThread),			\
		       AUXILLARY_THREAD_INIT_PRIORITY) {		\
    Thread *th = new Thread(startup, arg);				\
    if (th != NULL) {							\
      th->deref();							\
    }									\
    return th==NULL;							\
  }

#define SHUTDOWNTHREAD(shutdown) DEFINE_SHUTDOWN_FUNCTION(UNIQUE_IDENTIFIER(ShutdownThread), AUXILLARY_THREAD_SHUTDOWN_PRIORITY) { shutdown(); }

#endif // TEXTUS_BASE_THREAD_H_
