/* LeakyBucket.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 */

#include "textus/base/LeakyBucket.h"

namespace textus { namespace base {

uint32_t leakyTimeFunc() {
  return time(NULL);
}

}} //namespace
