 /* Key.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * Abstracts an assymetric key
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

#ifndef TEXTUS_KEYS_KEY_H_
#define TEXTUS_KEYS_KEY_H_

#include "textus/base/Base.h"
#include "textus/keys/Key.pb.h"

#include <typeinfo>

#define RSA_PK_TYPE 1
#define DEFAULT_KEY_TYPE RSA_PK_TYPE
#define REGISTER_KEY_PRIORITY 200

namespace textus { namespace keys {

class EncryptedKey;

class Key: virtual public textus::base::Base {

public:
  typedef Key *(*KeyFactory)(const char *); // Null means create new key.


  typedef int (*KeyIdentifier)(const char *); // figure out the type of this key.

  static Key *keyFromKeyPB(const KeyPB *);
  static Key *createKey(int type);
  static Key *fromString(const char *);
  static int registerKeyType(int type, KeyFactory factory);
  static int registerKeyIdentifier(KeyIdentifier ident);

  Key() {}
  virtual ~Key() {}

  virtual EncryptedKey *trust(Key *target) const; /* Owner of this key trusts the target.
						   * Encrypts the private key with the
						   * targets public key, and returns the
						   * result key.
						   */

  virtual KeyPB *protoBuf() const = 0;
  virtual std::string toString();

  virtual size_t encryptPublic(void *src, void *dst, size_t slen, size_t dlen) const = 0;
  virtual size_t decryptPrivate(void *src, void *dst, size_t slen, size_t dlen) const = 0;

  virtual size_t decryptPublic(void *src, void *dst, size_t slen, size_t dlen) const = 0;
  virtual size_t encryptPrivate(void *src, void *dst, size_t slen, size_t dlen) const = 0;

  virtual size_t encryptedSize(size_t src_size) const =0;
  virtual size_t decryptedSize(size_t src_size) const =0;

  virtual int cmp(const Key &key2) const = 0;

  bool operator ==(const Key &k2) const {
    if (typeid(*this) == typeid(k2)) {
      return cmp(k2) == 0;
    }
    return false;
  }
};

}} //namespace
#endif  //TEXTUS_KEYS_KEY_H_
