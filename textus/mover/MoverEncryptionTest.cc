/* MoverEncryptionTest.cc -*- c++ -*-
 * Copyright (c) 2013, 2014
 *
 * Test the MoverEncryption class.  In particular
 * the ExternalEncryption Methods.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/mover/MoverEncryption.h"
#include "textus/system/Environment.h"
#include "textus/file/Directory.h"

using namespace textus::testing;
using namespace textus::mover;

namespace textus { namespace mover {
using namespace std;

class MoverEncryptionTest: public UnitTest {
public:
  MoverEncryptionTest();
  ~MoverEncryptionTest();

  void run_tests();
  
};

MoverEncryptionTest::MoverEncryptionTest(): UnitTest(__FILE__)
{

  fprintf(stderr, "Testing Loading of ExternalEncryptions...");
  getDataDir();
  string config_path =
    textus::system::Environment::getConfigPath("crypto.cfg");
  
  assert(MoverEncryption::fromConfigFile(config_path) == 0);
  fprintf(stderr, "ok.\n");

  for ( MoverEncryption::iterator i = MoverEncryption::begin();
	i != MoverEncryption::end(); ++i) {
    string test_text = string ("This is a test.  This is only a test.\n");
    string out;
    fprintf (stderr, "Testing encryption %s...", i->first.c_str());
    MoverEncryption *me = MoverEncryption::findEncryption(i->first);
    assert(me != NULL);
    KeyDescription *key = me->tempKey();
    assert(key != NULL);
    assert(me->newKey(key) == 0);
    string e;
    assert (me->encrypt(key, test_text, &e) == 0);
    assert (e != test_text);
    string d;
    string key_name;
    assert (me->decrypt(key, e, &d, &key_name) == 0);
    assert (d == test_text);
    assert (key_name == key->at(me->keyName()));
    fprintf (stderr, "ok.\n");
    
  }

}

MoverEncryptionTest::~MoverEncryptionTest()
{
}

void MoverEncryptionTest::run_tests() {
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new MoverEncryptionTest();
}
