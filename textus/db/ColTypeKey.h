/* ColTypeKey.h -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Encapsulate a key in the database.
 * We'll use a string so that 
 * people may be able to identify and extract it.
 */

#ifndef TEXTUS_DB_COLTYPEKEY_H_
#define TEXTUS_DB_COLTYPEKEY_H_

#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/keys/Key.h"

namespace textus { namespace db {

using namespace textus::keys;

class ColTypeKey: public ColLobBase<SQL_VARCHAR> {
public:
  ColTypeKey() {}
  virtual ~ColTypeKey() {}
  
  Key *value() const {
    return Key::fromString(copyOut().c_str());
  }

  void setValue(Key *k) {
    copyIn(k->toString());
  }

  string sqlType() {
    string s = "varchar";
    return s;
  }

};

}} //namespace


#endif //TEXTUS_DB_COLTYPEKEY_H_
