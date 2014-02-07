/* KeyTest.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Test the Key class
 */

#include "textus/testing/UnitTest.h"
#include "textus/keys/Key.h"
#include "textus/keys/EncryptedKey.h"

using namespace textus::testing;
using namespace textus::keys;

namespace textus { namespace keys {

class KeyTest: public UnitTest {
private:

public:
  KeyTest();
  ~KeyTest();

  void run_tests();
};

KeyTest::KeyTest():UnitTest(__FILE__)
{
}

KeyTest::~KeyTest()
{
}

void KeyTest::run_tests() {
  fprintf (stderr, "Testing key creation...");
  Key *key1 = Key::createKey(RSA_PK_TYPE);
  assert(key1 != NULL);
  Key *key2 = Key::createKey(RSA_PK_TYPE);
  assert(key2 != NULL);
  fprintf(stderr, "ok\n");

  fprintf (stderr, "Testing key encryption...");
  EncryptedKey *ek1 = key1->trust(key2); // key1 trusts key2.
  assert(ek1 != NULL);
  fprintf (stderr, "ok\n");
  
  fprintf (stderr, "Testing key decryption...");
  Key *key3 = ek1->useTrust(key2); // use the fact that key1 trusted key2
  assert (key3 != NULL);
  assert (*key3 == *key1);
  fprintf (stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new KeyTest();
}
