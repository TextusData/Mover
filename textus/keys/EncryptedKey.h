 /* EncryptedKey.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a key that has been encrypted.
 *
 */

/*
 *   This program is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 3 of the
 *   License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see
 *   <http://www.gnu.org/licenses/>.
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
