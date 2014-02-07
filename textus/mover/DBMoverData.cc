/* DBMoverData.cc -*- c++ -*-
 * Copyright (C) 2013 Ross Biro
 *
 */

#include "textus/mover/DBMoverData.h"
#include "textus/db/DBType.h"
#include "textus/db/ColTypeUUID.h"

namespace textus { namespace mover {

STATIC_DB_DECL(DBMoverData);

string DBMoverData::table() {
  return string("mover_hashes");
}

}} //namespace
