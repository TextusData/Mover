/* DBConnectTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the DBInstance class and make a connection.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/db/DBStatement.h"
#include "textus/db/DBHandle.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

namespace textus { namespace db {

class DBConnectTest: public UnitTest {
private:

public:
  DBConnectTest();
  ~DBConnectTest();

  void run_tests();
};

DBConnectTest::DBConnectTest(): UnitTest(__FILE__)
{
}

DBConnectTest::~DBConnectTest()
{
}

void DBConnectTest::run_tests() {
  AUTODEREF(DBInstance *, instance);
  fprintf(stderr, "Creating Test Instance...");
  instance = new DBInstance();
  assert(instance!= NULL);
  instance->setServer(getDSN());
  instance->setUser("unittest");
  fprintf(stderr, "ok\n");

  fprintf(stderr, "Connecting...");
  assert (instance->connect() == 0);
  fprintf (stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new DBConnectTest();
}
