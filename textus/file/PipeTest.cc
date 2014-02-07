/* PipeTest.cc -*- c++ -*-
 * Copyright (c) 2010
 *
 * Test the Pipe class.
 */
#include "textus/testing/UnitTest.h"
#include "textus/file/Pipe.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {

class PipeTest: public UnitTest {
public:
  PipeTest();
  ~PipeTest();

  void run_tests();
  
};

PipeTest::PipeTest(): UnitTest(__FILE__)
{
}

PipeTest::~PipeTest()
{
}

void PipeTest::run_tests() {
  string test("test");
  fprintf(stderr, "Testing pipes...");
  AutoDeref<Pipe> p;
  p = new Pipe();
  p->deref();
  p->writer()->write(test);
  string r = p->reader()->read(20);
  assert (r == test);
  fprintf(stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new PipeTest();
}
