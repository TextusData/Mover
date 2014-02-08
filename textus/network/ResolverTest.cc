/* ResolverTest.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Test the Resolver class in NetworkAddress.
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
