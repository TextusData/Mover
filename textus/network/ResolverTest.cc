/* ResolverTest.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Test the Resolver class in NetworkAddress.
 */

#include "textus/base/Base.h"
#include "textus/testing/UnitTest.h"
#include "textus/network/NetworkAddress.h"

using namespace textus::testing;
using namespace textus::network;

namespace textus { namespace network {

class ResolverTest: public UnitTest {
private:

public:
  ResolverTest();
  ~ResolverTest();

  void run_tests();
};

ResolverTest::ResolverTest(): UnitTest(__FILE__)
{
}

ResolverTest::~ResolverTest()
{
}

void ResolverTest::run_tests() {
  for (int i = 0; i < 100; i++ ) {
    fprintf (stderr, "Testing Resolver...");
    AUTODEREF(NetworkAddress *, na );
    na = NetworkAddress::resolve("www.google.com");
    na->waitForNameResolution();
    fprintf (stderr, "ok\n");
  }
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ResolverTest();
}
