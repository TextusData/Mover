/* LineReaderTest.cc -*- c++ -*-
 * Copyright (c) 2010
 *
 * Test the LineReader class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/file/FileHandle.h"
#include "textus/file/LineReader.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {

class LineReaderTest: public UnitTest {
public:
  LineReaderTest();
  ~LineReaderTest();

  void run_tests();
  
};

LineReaderTest::LineReaderTest(): UnitTest(__FILE__)
{
}

LineReaderTest::~LineReaderTest()
{
}

void LineReaderTest::run_tests() {
  for (int i = 0; i < 10; i++ ) {
    AUTODEREF(TextusFile *, foo);
    fprintf(stderr, "Testing LineReader...");
    AUTODEREF(Directory *, tmpdir);
    tmpdir = getTmpDir();
    assert(tmpdir != NULL);
    tmpdir->ref();
    foo = tmpdir->openFile("line-reader-test", O_WRONLY|O_CREAT);
    assert(foo != NULL);
    string wf=string("This is a line reader only a test.\n");

    foo->write(wf);
    foo->close();;
    foo->deref();

    foo = tmpdir->openFile("line-reader-test", O_RDONLY);
    assert(foo != NULL);
    FileHandle *fh = dynamic_cast<FileHandle *>(foo);
    AUTODEREF(LineReader *, lr);
    lr = new LineReader(fh);
    foo->deref();
    foo = NULL;

    string l = lr->waitForLine();
    assert (l == "This is a line reader only a test.");
    fprintf(stderr, "ok\n");
  }
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new LineReaderTest();
}
