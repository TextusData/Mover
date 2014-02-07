/* Time.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * A class that represents the time.
 */
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "textus/base/Time.h"


namespace textus { namespace base {

Time Time::now() 
{
  struct tm tm;
  time_t n;
  time(&n);
  localtime_r(&n, &tm);
  Time t(&tm);
  return t;
}

}} //namespace
