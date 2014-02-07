/* ColTypeUUID.h -*- c++ -*-
 * Copyright (c) 2012, 2013 Ross Biro
 * Contains the base classes that represents a uuid as stored in a database.
 * We are storing UUID's as strings to that they might be human readable.
 *
 */

#include "textus/base/UUID.h"
#include "textus/db/ColType.h"

#ifndef TEXTUS_DB_COLTYPEUUID_H_
#define TEXTUS_DB_COLTYPEUUID_H_

namespace textus { namespace db {

using namespace textus::base;

class ColTypeUUID: public DBStringBase<36> {
public:
  ColTypeUUID() {}
  virtual ~ColTypeUUID() {}

  UUID value() const {
    UUID uu(copyOut());
    return uu;
  }

  void setValue(const UUID u) {
    copyIn(u.toString());
  }

  void generate() {
    UUID uu;
    uu.generate();
    setValue(uu);
  }
};

}} // namespace

#endif
