/* MoverMessage.cc -*- c++ -*-
 * Copyright (c) Ross Biro
 *
 */

#include "textus/base/init/Init.h"
#include "textus/mover/Mover.h"
#include "textus/mover/MoverMessage.h"
#include "textus/mover/MoverMessageProcessor.h"

namespace textus { namespace mover {

DEFINE_INT_ARG(mover_random_data_max, 16384, "mover_random_data_max", "The maximum size of the random chunk that is incl*uded in every message.");
DEFINE_INT_ARG(mover_random_data_min, 4096, "mover_random_data_min", "The minimum size of the random chunk that is included in every message.");

}} //namespace

