/* ThreadID.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Encapsulate the idea of a thread.
 */

#ifndef TEXTUS_BASE_THREADID_H_
#define TEXTUS_BASE_THREADID_H_

#include "textus/base/Utility.h"

#include <string.h>

#include <ostream>
#include <pthread.h>


namespace textus { namespace base {
  class ThreadID /* lightweight class, no base */ {
  private:
    friend class Thread;
    friend std::ostream& operator<< (std::ostream& o, const ThreadID &tid);

    pthread_t id;

  public:
    ThreadID()
    {
      // id is a pointer, so this will make it null.
      memset (&id, 0, sizeof(id));
    }

    static ThreadID self() {
      ThreadID d;
      d.id = pthread_self();
      return d;
    }

    static ThreadID invalid() {
      ThreadID d;
      return d;
    }

    ~ThreadID()
    {
    }


    bool operator == (const ThreadID &i2) const
    {
      return pthread_equal(id, i2.id);
    }

    bool operator < (const ThreadID &i2) const
    {
      return id < i2.id;
    }

    COMPARISON_OPERATORS(ThreadID);

  };
}} //namespace

#endif //TEXTUS_BASE_THEADID_H_
