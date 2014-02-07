/* OneTimeUse.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Template for one time use thingies.  Needs a lot of integer like properties to work
 * properly.
 */

#include "textus/base/OneTimeUse.h"
namespace textus { namespace base {

DEFINE_INT_ARG(one_time_chunk_default_reserve, 16, "one_time_chunk_default_reserve", "How big of chunks to reserve for each thread for one time use variables.");

}} //namespace
