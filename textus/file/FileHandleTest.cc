/* FileHandleTest.cc -*- c++ -*-
 * Copyright (c) 2009-2013
 *
 * Test the FileHandle class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/file/TextusFile.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {

class FileHandleTest: public UnitTest {
public:
  FileHandleTest();
  ~FileHandleTest();

  void run_tests();
  
};

FileHandleTest::FileHandleTest(): UnitTest(__FILE__)
{
}

FileHandleTest::~FileHandleTest()
{
}

void FileHandleTest::run_tests() {
  fprintf(stderr, "Testing file open...");
  AUTODEREF(Directory *, tmpdir);
  tmpdir = getTmpDir();
  assert(tmpdir != NULL);
  tmpdir->ref();
  FileHandle *foo = tmpdir->openFile("file-test", O_WRONLY|O_CREAT);
  assert(foo != NULL);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Testing file write...");
  string wf=string("This is only a test.\n");

  foo->write(wf);
  fprintf(stderr, "ok\n");

  fprintf(stderr, "Testing file close...");
  foo->close();
  fprintf(stderr, "ok\n");

  foo->deref();

  fprintf(stderr, "Testing file open read only...");
  foo = tmpdir->openFile("file-test", O_RDONLY);
  assert(foo != NULL);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Testing file read...");
  string red = foo->read(1024);
  assert (red == wf);
  fprintf(stderr, "ok\n");

  foo->close();

  foo->deref();

  fprintf(stderr, "Testing file buffered write...");
  foo = tmpdir->openFile("file-test", O_RDWR);
  wf=string("This was only a test.\n");
  foo->closeOnEmptyBuffer();
  foo->addToOutBuffer(wf);
  foo->waitForClose();
  foo->deref();

  foo = tmpdir->openFile("file-test", O_RDONLY);
  assert(foo != NULL);
  red = foo->read(1024);
  assert (red == wf);
  fprintf(stderr, "ok\n");
  foo->close();
  foo->deref();
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new FileHandleTest();
}
