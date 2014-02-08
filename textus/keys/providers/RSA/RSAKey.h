/* RSAKey.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 * Uses RSA for an assymetric key
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

#ifndef TEXTUS_KEYS_PROVIDERS_RSA_RSAKEY_H_
#define TEXTUS_KEYS_PROVIDERS_RSA_RSAKEY_H_

#include "textus/base/Base.h"
#include "textus/keys/Key.h"

#include <string>
namespace textus { namespace keys { namespace providers { namespace RSA {

using namespace std;

extern "C" {
#include "openssl/rsa.h"
}

class RSAKey: public Key {
private:
  textus::base::AutoFree<RSA> key;

  explicit RSAKey(const string d);
  explicit RSAKey(const char *);
  explicit RSAKey(BUF_MEM *);
  explicit RSAKey(RSA *k): key(k) {}
  explicit RSAKey() {}

public:
  static Key *createRSAKey(const char *);
  virtual ~RSAKey() {}

  virtual KeyPB *protoBuf() const;

  virtual size_t encryptPublic(void *src, void *dst, size_t src_len, size_t dst_len ) const;
  virtual size_t decryptPublic(void *src, void *dst, size_t src_len, size_t dst_len ) const;
  virtual size_t encryptedSize(size_t src_size) const;

  virtual size_t decryptPrivate(void *src, void *dst, size_t src_len, size_t dst_len) const;
  virtual size_t encryptPrivate(void *src, void *dst, size_t src_len, size_t dst_len) const;
  virtual size_t decryptedSize(size_t src_size) const;

  virtual int cmp(const Key &k2) const;
};
}}}} // namespace
#endif //TEXTUS_KEYS_PROVIDERS_RSA_RSAKEY_H_
