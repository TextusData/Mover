/* Base.h -*- C++ -*-
 * Copyright (C) 2009-2014 Ross Biro
 * A Base class for everything to inherit from.
 * Do not add values if it can be avoided.
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

#ifndef TEXTUS_BASE_BASE_H_
#define TEXTUS_BASE_BASE_H_

#include <stddef.h>

#include "textus/base/Debug.h"
#include "textus/system/OS.h"
#include "textus/base/locks/Mutex.h"
#include "textus/base/locks/Condition.h"
#include "textus/base/logic/Bool.h"
#include "textus/base/ThreadID.h"
#include "textus/base/AutoFree.h"
#include "textus/base/Compiler.h"
#include "textus/base/Members.h"

#include <pthread.h>
#include <stdlib.h>


namespace textus { namespace base {
using namespace textus::base::locks;
using namespace std;

extern bool testing;

// This needs to be at the top since we use it in some of the inline functions below.
#define UNIQUE_IDENTIFIER__(name, l, c) name ## _ ## l ## _ ## c ## _ ## name
#define UNIQUE_IDENTIFIER_(name, l, c) UNIQUE_IDENTIFIER__(name, l, c)
#define UNIQUE_IDENTIFIER(name) UNIQUE_IDENTIFIER_(name, __LINE__, __COUNTER__)

#define DEREF_MAGIC   0x0FACEBAB

class Base {
#ifdef TESTING
  friend class BaseTest;
  friend class CacheTest;
#endif

private:
  static int instance_count;

  int ref_count;
  int weak_ref_count;

  Mutex mutex;
  Condition condition;
  ThreadID sync_thread_id;
  int sync_count;
  
  friend std::ostream& operator << (std::ostream& o, const Base& base);
  friend std::ostream& operator << (std::ostream& o, const Base * const base);
  friend class Synchronizer; 

  Base(const Base &old)
  {
    LOG(ERROR) << "Base copy constructor called.  This is not allowed to prevent the"
      "compiler from generating all sorts of bad code.\n";
    LOG_CALL_STACK(ERROR);
    die();
  }

  Base &operator = (const Base &b2)
  {
    LOG(ERROR) << "Base copy operator called.  This is not allowed to prevent the"
      "compiler from generating all sorts of bad code.\n";
    LOG_CALL_STACK(ERROR);
    die();
    return *this;
  }

  std::ostream &doPrint(std::ostream &o) const;

protected:
  virtual int synchronize()
  {
    LOCK(&mutex);
    mb();
    while (sync_count > 0) {
      assert(sync_thread_id != ThreadID::invalid());
      if (sync_thread_id == ThreadID::self()) {
	break;
      }
      if (!threadWaiting(&mutex)) {
	condition.wait(mutex);
      }
      mb();
    }
    mb();
    if (0 == sync_count++) {
      sync_thread_id = ThreadID::self();
#ifdef DEBUG_LOCKS
      if (lock_level != INT_MAX) { // ignore locks with the default.
	int cll = currentLockLevel();
	if (lock_level > cll) {
	  fprintf(stderr, "lock_level = %d, cll = %d\n", lock_level, cll);
	}
	assert(lock_level <= currentLockLevel());
	pushLockLevel(lock_level);
      }
#endif
    }
    mb();
    assert(sync_thread_id == ThreadID::self());
    return sync_count;
  }


#ifdef DEBUG_LOCKS
  int lock_level;
  int currentLockLevel() const;
  void pushLockLevel(int) const;
  void popLockLevel() const;
#endif

  virtual void syncReleased() { }

  bool haveSync()
  {
    LOCK(&mutex);
    if (sync_count == 0) {
      return false;
    }
    return sync_thread_id == ThreadID::self();
  }

  // Override this to clean up the weak references.
  virtual int weakDelete()
  {
    return weak_ref_count == 0;
  }

public:
  Base():ref_count(1), weak_ref_count(0), condition(), sync_thread_id(), sync_count(0)
  {
#ifdef DEBUG_LOCKS
    lock_level = INT_MAX;
#endif
    instance_count++;
  }

  virtual ~Base();

#ifdef DEBUG_LOCKS
  void setLockLevel(int level) {
    lock_level = level;
  }
#endif
  virtual std::string toString() const;
  
  operator std::string() const {
    return toString();
  }

  int trySync()
  {
    LOCK(&mutex);
    if (sync_count == 0 || sync_thread_id == ThreadID::self()) {
      sync_count++;
      sync_thread_id = ThreadID::self();
      mb();
      return 1;
    }
    return 0;
  }

  virtual int desynchronize()
  {
    bool released=false;
    {
      LOCK(&mutex);
      mb();
      assert(sync_thread_id == ThreadID::self());
      if (--sync_count == 0) {
#ifdef DEBUG_LOCKS
	if (lock_level != INT_MAX) { 
	  popLockLevel();
	}
#endif
	sync_thread_id = ThreadID::invalid();
	condition.signal();
	released = true;
      }
      mb();
      if (sync_count == 0) {
	assert( sync_thread_id == ThreadID::invalid() );
      }
    }
    if (released) {
      syncReleased();
    }
    return sync_count;
  }

  void wait();
  virtual void signal();

  void block() {
    synchronize();
  }

  void unblock() {
    assert(sync_thread_id == ThreadID::self());
    assert(sync_count > 0);
    desynchronize();
  }

  int weakRef()
  {
    LOCK(&mutex);
    return ++weak_ref_count;
  }

  int weakDeref()
  {
    // Can't use LOCK here, the delete this
    // will delete the lock, and then attempt
    // to unlock it.
    mutex.lock();
    if (--weak_ref_count == 0) {
      if (ref_count == 0) {
	ref_count = DEREF_MAGIC;
	mutex.unlock();
	delete this;
	return 1;
      }
    }
    mutex.unlock();
    return 0;
  }

  int ref()
  {
    LOCK(&mutex);
    return ++ref_count;
  }

  int deref() 
  {
    // Can't use LOCK here, the delete this
    // will delete the lock, and then attempt
    // to unlock it.
    mutex.lock();
    int loop_count=0;
    //assert(ref_count > 0 && ref_count < 1000);
    assert(ref_count > 0);
    while (--ref_count == 0) {
      loop_count++;
      if (loop_count > 900) {
	LOG(ERROR) << "weakderef loop count exceeded 900\n";
      }
      assert(loop_count < 1000);
      // Make sure the weak delete doesn't delete us.
      // Ref count is 0 here, let's make it big to detect
      // something else messing with us.
      assert (ref_count == 0);
      ref_count = 1;
      mutex.unlock();
      if (weak_ref_count == 0 || weakDelete()) {
	mutex.lock();
	// See if we have been ressurected.
	if (ref_count > 1 || weak_ref_count != 0) {
	  continue;
	}
	assert( ref_count == 1);
	ref_count = DEREF_MAGIC;
	mutex.unlock();
	delete this;
	return 1;
      }
    }
    mutex.unlock();
    return 0;
  }
};

}} // namespace
// Needs Base to be defined first.
#include "textus/base/AutoDeref.h"

namespace textus { namespace base { namespace init {
extern volatile bool shutdown_done;
extern volatile bool shutdown_started;
}

class Holder /* Lightweight no base. */ {
protected:
  Base *obj;

public:
  Holder(Base *o): obj(o)
  {
    if (!textus::base::init::shutdown_done)  {
      obj->ref();
    }
  }

  ~Holder()
  {
    if (!textus::base::init::shutdown_done)  {
      obj->deref();
    }
  }
};

class Synchronizer: public Holder {
protected:

public:
  Synchronizer(Base *o): Holder(o)
  {
    if (!textus::base::init::shutdown_done)  {
      obj->synchronize();
    }
  }

  ~Synchronizer() 
  {
    if (!textus::base::init::shutdown_done)  {
      obj->desynchronize();
    }
  }
};


template <class T> class AutoDerefer /* Lightweight no base. */ {
private:
  T *obj;

public:
  AutoDerefer(T *o): obj(o) {}
  ~AutoDerefer() { if (*obj) (*obj)->deref(); }
    
};

template <class T> class AutoDeleter /* Lightweight no base. */ {
private:
  T *obj;

public:
  AutoDeleter(T *o): obj(o) {}
  ~AutoDeleter() { if (*obj) delete *obj; }
};

class AutoFreer /* Lightweight no base. */ {
private:
  void **obj;

public:
  AutoFreer(void **o): obj(o) {}
  ~AutoFreer() { if (*obj) free(*obj); }
};

//foo must be an object that derives from Base.  This goes inside the
// braces, unlike java, and synchronizes (or holds a reference) to the
// end of the scope.  beware of goto with this.

#define SYNCHRONIZED(foo) textus::base::Synchronizer UNIQUE_IDENTIFIER(SYNC)( (foo) )
#define Synchronized(foo) SYNCHRONIZED(foo)
#define SynchronizedConst(foo) SYNCHRONIZED(const_cast<Base *>(static_cast<const Base *>(foo)))

#define HOLD(foo)  textus::base::Holder UNIQUE_IDENTIFIER(HOLD)( (foo) );

#define AUTODEREF(type, name)  type name = NULL; textus::base::AutoDerefer<type> UNIQUE_IDENTIFIER(DEREFER)(&name);
//#define AUTODEREF(type, name) AutoDeref<type> name;

#define AUTODELETE(type, name) type name = NULL; textus::base::AutoDeleter<type> UNIQUE_IDENTIFIER(DELETER)(&name);

#define AUTOFREE(type, name) type name = NULL; textus::base::AutoFreer UNIQUE_IDENTIFIER(FREER)(&name);

#define HRC(res) do { ret = (res); if (ret) { LOG(INFO) << "Error at " << __FILE__ << ", " << __LINE__ << " in statement: " #res " ( = " << ret << ")\n"; goto error_out; } } while (0)
#define HRI(res) do { ret = (res); if (ret) { LOG(INFO) << "Error at " << __FILE__ << ", " << __LINE__ << " in statement: " #res " ( = " << ret << ")[Ignored]\n"; } } while (0)
#define HRNULL(res) do {if (!(res)) { LOG(INFO) << "Error at " << __FILE__ << ", " << __LINE__ << " in statement: " #res "  (= NULL)" << "\n"; ret = -1 ;goto error_out; }} while (0)
#define HRZ(res) HRC(!(res))
#define HRTRUE(res) HRC(!(res))
#define HRFALSE(res) HRC(res)


// Poor man's wait.  Uses the same mutex/condition as the
// synchronize code, so it will get woken up everytime a
// synchronize is left.
inline void Base::wait() {
  Synchronized(this);
  { // use to enforce the order of release.  First the mutex, then the sync.
    LOCK(&mutex);
    ThreadID sid = sync_thread_id;
    int scount = sync_count;
    sync_thread_id = ThreadID::invalid();
    sync_count = 0;
    condition.signal();
    do {
      condition.wait(mutex);
      mb();
    } while (sync_count > 0);
    assert(sync_count == 0);
    sync_count = scount;
    sync_thread_id = sid;
    mb();
  }
}

inline void Base::signal() {
  LOCK(&mutex);
  mb();
  condition.signal();
}

}} // namespace base

#include "textus/base/AutoDeref.h"


#endif //TEXTUS_BASE_BASE_H_
