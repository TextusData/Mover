/* DBPoolTest.cc -*- c++ -*-
 * Copyright (c) 2012 Ross Biro
 *
 * Test the DBPool class and make a connection.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/db/ColTypeUUID.h"
#include "textus/db/DBStatement.h"
#include "textus/db/DBHandle.h"

using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

#define MAX_ROWS 100

namespace textus { namespace db {

class TestDB: public DBType {
  BEGIN_DB_DECL(TestDB);
  DECL_DB_ENTRY(TestDB, string, DBString<64>, name, 0);
  DECL_DB_ENTRY(TestDB, int, DBInt, value, 0);
  END_DB_DECL(TestDB);

public:
  TestDB() {}
  virtual ~TestDB() {}

  string table() { return "test_table"; }

};

STATIC_DB_DECL(TestDB);

class DBPoolTest: public UnitTest {
private:
  const char *dbScript() { return "$(APP_PATH)/db/create_db_script"; }

public:
  DBPoolTest();
  ~DBPoolTest();

  void run_tests();
};

DBPoolTest::DBPoolTest(): UnitTest(__FILE__)
{
}

DBPoolTest::~DBPoolTest()
{
}

void DBPoolTest::run_tests() {
  AUTODEREF(DBPool *, pool);
  int ret=0;
  fprintf(stderr, "Creating Test Pool...");
  pool = DBPool::getPool(); // use the static pool for testing.
  assert(pool != NULL);
  fprintf(stderr, "ok\n");

  fprintf(stderr, "Checking for test_table....");
  TestDB *tdb = NULL;
  tdb = TestDB::allocResultSet(MAX_ROWS);
  assert (tdb != NULL);
  ret = pool->query(string("select name, value from test_table"), MAX_ROWS, tdb);
  assert(ret > 0);
  fprintf (stderr, "ok\n");
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new DBPoolTest();
}
