/* EncryptedKey.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a key that has been encrypted.
 *
 */


#include "textus/keys/EncryptedKey.h"
#include "textus/keys/Key.h"

namespace textus { namespace keys {

EncryptedKeyPB *EncryptedKey::protoBuf() const
{
  EncryptedKeyPB *pb = new EncryptedKeyPB();
  pb->set_data(buffer);
  return pb;
}

Key *EncryptedKey::useTrust(Key *target) const
{
  size_t buffer_size = target->decryptedSize(buffer.length());
  char *buf = static_cast<char *>(alloca(buffer_size));
    
  int ret = target->decryptPrivate(const_cast<char *>(buffer.c_str()), buf, buffer.length(), buffer_size);

  if (ret < 0) {
    return NULL;
  }

  buffer_size = ret;

  string d(buf, buffer_size);
  AUTODELETE(KeyPB *, pb);
  pb = new KeyPB();
  pb->set_data(d);
  return Key::keyFromKeyPB(pb);
}


}} //namespace
