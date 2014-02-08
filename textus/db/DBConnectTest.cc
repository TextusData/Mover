/* DBConnectTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the DBInstance class and make a connection.
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
