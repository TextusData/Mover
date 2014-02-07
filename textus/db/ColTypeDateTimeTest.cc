/* ColTypeDateTimeTest.cc -*- c++ -*-
 * Copyright (c) 2012-2013 Ross Biro
 *
 * Test the ColTypeDateTime column type.
 */

#include "textus/testing/UnitTest.h"
#include "textus/base/Base.h"
#include "textus/db/ColType.h"
#include "textus/db/ColTypeDateTime.h"
#include "textus/db/ColTypeUUID.h"
#include "textus/db/DBHandle.h"
#include "textus/base/UUID.h"

#include <time.h>


using namespace textus::testing;
using namespace textus::base;
using namespace textus::db;

namespace textus { namespace db {

class TestDB: public DBType {
  BEGIN_DB_DECL(TestDB);
  DECL_DB_ENTRY(TestDB, UUID, ColTypeUUID, pk, DT_PRIMARY_KEY);
  DECL_DB_ENTRY(TestDB, Time *, ColTypeDateTime, time, 0);
  END_DB_DECL(TestDB);

public:
  TestDB() {}
  virtual ~TestDB() {}

  string table() { return "test_table"; }

};


STATIC_DB_DECL(TestDB);

class ColTypeDateTimeTest: public UnitTest {
private:
  const char *dbScript() { return "$(APP_PATH)/db/create_db_script"; }

public:
  ColTypeDateTimeTest();
  ~ColTypeDateTimeTest();

  void run_tests();
};

ColTypeDateTimeTest::ColTypeDateTimeTest(): UnitTest(__FILE__)
{
}

ColTypeDateTimeTest::~ColTypeDateTimeTest()
{
}

void ColTypeDateTimeTest::run_tests() {
  AUTODEREF(DBInstance *, instance);
  fprintf(stderr, "Creating Test Instance...");
  instance = new DBInstance();
  assert(instance!= NULL);
  instance->setServer(getDSN());
  instance->setUser("unittest");
  fprintf(stderr, "Connecting...");

  assert (instance->connect() == 0);
  assert(instance->getHandle() != NULL);
  fprintf (stderr, "ok\n");

  fprintf(stderr, "Adding row to test table....");
  char buff[1024];
  struct tm tm;
  time_t now;
  time(&now);

  strftime(buff, sizeof(buff), "Insert into test_table (pk, time) values ('000000', '%F %r+0');",
	   gmtime_r(&now, &tm));

  localtime_r(&now, &tm); // get the timezone info

  string sql(buff);

  
  AUTODEREF(DBStatement *, stmt);
  stmt = instance->getHandle()->prepare(sql);
  assert (stmt != NULL);
  assert(stmt->execute() == 0);
  fprintf (stderr, "ok\n");

  TestDB tdb;

  fprintf(stderr, "executing query....");
  int res = TestDB::query("select * from test_table", 1, &tdb, instance->getHandle());
  assert (res == 1);
  fprintf (stderr, "ok\n");

  fprintf (stderr, "Checking to see if we got the same time back...");
  Time *t1;
  t1 = tdb.time();

  Time *t2;
  t2 = new Time(&tm);

  LOG(DEBUG) << "t1=" << t1 << " and t2=" << t2 << endl;
  assert (*t2 == *t1);
  fprintf (stderr, "ok\n");
  delete t1;
  delete t2;
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ColTypeDateTimeTest();
}
