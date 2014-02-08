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
#include "textus/base/Base.h"
#include "textus/base/init/Init.h"
#include "textus/keys/Key.h"
#include "textus/keys/EncryptedKey.h"
#include "textus/keys/Key.pb.h"
#include "textus/base/locks/ReadWriteLock.h"
#include "textus/base/locks/ReadLock.h"

#include <map>

namespace textus { namespace keys {

using namespace std;
using namespace textus::base::locks;

static  map <int, Key::KeyFactory > keyFactories;
static ReadWriteLock factoryLock;
static list<Key::KeyIdentifier> keyIdentifiers;

EncryptedKey *Key::trust(Key *target) const
{
  AUTODELETE(KeyPB *, pb);
  pb = protoBuf();

  size_t epkl = target->encryptedSize(pb->data().length());
  char *encryptedPrivKey = static_cast<char *>(alloca(epkl));
    
  target->encryptPublic(const_cast<char *>(pb->data().c_str()), encryptedPrivKey, pb->data().length(), epkl);

  return new EncryptedKey(string (encryptedPrivKey, epkl));
}

string Key::toString() {
  KeyPB *pb = protoBuf();
  return pb->data();
}

Key *Key::keyFromKeyPB(const KeyPB *pb) {
  int type = DEFAULT_KEY_TYPE;
  if (pb == NULL || !pb->has_data()) {
    return NULL;
  }
   
  if (pb->has_type()) {
    type = pb->type();
  }

  ReadLock rl(&factoryLock);

  if (keyFactories.find(type) == keyFactories.end()) {
    return NULL;
  }

  return keyFactories[type](pb->data().c_str());
}


int Key::registerKeyType(int type, Key::KeyFactory factory)
{
  WriteLock wl(&factoryLock);
  if (keyFactories.find(type) != keyFactories.end()) {
    return -1;
  }
  keyFactories[type] = factory;
  return 0;
}

Key *Key::createKey(int type) {
  ReadLock rl(&factoryLock);

  if (keyFactories.find(type) == keyFactories.end()) {
    return NULL;
  }

  return keyFactories[type](NULL);
}

int Key::registerKeyIdentifier(Key::KeyIdentifier ident) 
{
  keyIdentifiers.push_back(ident);
  return 0;
}

Key *Key::fromString(const char *c) {
  ReadLock rl(&factoryLock);

  // First see if anyone claims this string representation.
  for (list<KeyIdentifier>::iterator i = keyIdentifiers.begin(); i != keyIdentifiers.end(); ++i) {
    int t = (*i)(c);
    if (t > 0) {
      if (keyFactories.find(t) != keyFactories.end()) {
	return keyFactories[t](c);
      }
    }
  }

  // Last Resort.  Just try them all.  Hopefully all the key types sign their own string
  // representations some how.
  for (map<int, Key::KeyFactory>::iterator i = keyFactories.begin(); i != keyFactories.end(); ++i) {
    Key *k = (*i).second(c);
    if (k != NULL) {
      return k;
    }
  }

  return NULL;

}

// put this here to force the RSA stuff to load.
namespace providers { namespace RSA {

extern Key *createRSAKey(const char *);
extern int identifyRSAKey(const char *);

DEFINE_INIT_FUNCTION(RSAKeyInit, REGISTER_KEY_PRIORITY)
{
  int ret;
  HRC(Key::registerKeyType(RSA_PK_TYPE, createRSAKey));
  HRC(Key::registerKeyIdentifier(identifyRSAKey));

 error_out:
  return ret;
} 
 
}} // namespace


}} // namespace

