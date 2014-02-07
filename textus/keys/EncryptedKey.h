 /* EncryptedKey.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a key that has been encrypted.
 *
 */

#ifndef TEXTUS_KEYS_ENCRYPTED_KEY_H_
#define TEXTUS_KEYS_ENCRYPTED_KEY_H_

#include "textus/base/Base.h"
#include "textus/keys/EncryptedKey.pb.h"

#include <string>

namespace textus { namespace keys {

class Key;

using namespace std;

class EncryptedKey: virtual public textus::base::Base {
private:
  string buffer;

public:
  explicit EncryptedKey(string s): buffer(s) {}
  virtual ~EncryptedKey() {}

  EncryptedKeyPB *protoBuf() const;

  string data() const {
    return buffer;
  }

  Key *useTrust(Key *) const;
};

}} //namespace
#endif  //TEXTUS_KEYS_ENCRYPTED_KEY_H_
