/*ThreadID.cc -*- c++ -*-
 *Copyright (C) 2009 Ross Biro
 *
 * Lightweight class to contain a threadid.
 */
#include "textus/base/ThreadID.h"

#include <iostream>

namespace textus { namespace base {
    std::ostream& operator<< (std::ostream& o, const ThreadID& tid)
    {
      o << (void *)tid.id;
      return o;
    }
}} //namespace
