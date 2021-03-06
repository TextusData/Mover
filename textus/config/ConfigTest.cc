/* ConfigTest.cc -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
 *
 * Test the config file reader.
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
#include "textus/config/Config.h"
#include "textus/file/StringFile.h"

using namespace textus::testing;
using namespace textus::config;

namespace textus { namespace config {

class ConfigTest: public UnitTest {
private:

public:
  ConfigTest();
  ~ConfigTest();

  void run_tests();
};

ConfigTest::ConfigTest(): UnitTest(__FILE__)
{
}

ConfigTest::~ConfigTest()
{
}

void test1() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo = bar";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing single name/value pair....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("foo")) == 1);
  cd = (cd->asMap())[string("foo")];
  assert(cd != NULL);
  assert (cd->type() == ConfigData::sdata);
  assert(cd->asString() == string("bar"));
  fprintf(stderr, "ok\n");
}

void test2() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo= [bar, baz]";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing single name/list pair....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("foo")) == 1);
  cd = (cd->asMap())[string("foo")];
  assert(cd != NULL);
  assert (cd->type() == ConfigData::ldata);
  assert(cd->asList().size() == 2);
  assert(cd->asList().front()->asString() == string("bar"));
  assert(cd->asList().back()->asString() == string("baz"));
  fprintf(stderr, "ok\n");
}

void test3() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo ={bar = baz}";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing single name/map pair....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("foo")) == 1);
  cd = (cd->asMap())[string("foo")];
  assert(cd != NULL);
  assert (cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("bar")) == 1);
  cd = (cd->asMap())[string("bar")];
  assert(cd->asString() == string("baz"));
  fprintf(stderr, "ok\n");
}

void test4() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo = {bar = baz},\n\tfoo2 = {bar2 = baz2}";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing multiple name/map pair....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 2);
  assert(cd->asMap().count(string("foo")) == 1);
  ConfigData *c2 = (cd->asMap())[string("foo")];
  assert(c2 != NULL);
  assert (c2->type() == ConfigData::mdata);
  assert(c2->asMap().size() == 1);
  assert(c2->asMap().count(string("bar")) == 1);
  c2 = (c2->asMap())[string("bar")];
  assert(c2->asString() == string("baz"));
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 2);
  assert(cd->asMap().count(string("foo2")) == 1);
  c2 = (cd->asMap())[string("foo2")];
  assert(c2 != NULL);
  assert (c2->type() == ConfigData::mdata);
  assert(c2->asMap().size() == 1);
  assert(c2->asMap().count(string("bar2")) == 1);
  c2 = (c2->asMap())[string("bar2")];
  assert(c2->asString() == string("baz2"));
  fprintf(stderr, "ok\n");
}

void test5() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo = \" bar \\\"\"";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing single name/value pair with quotes....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("foo")) == 1);
  cd = (cd->asMap())[string("foo")];
  assert(cd != NULL);
  assert (cd->type() == ConfigData::sdata);
  assert(cd->asString() == string(" bar \""));
  fprintf(stderr, "ok\n");
}

void test6() {
  AUTODEREF(Config *, cfg);
  string config1 = "foo<$bar> = <$baz>\nvars={ bar=bar1, baz=baz1}\n$eval foo<$bar> vars\n$template foo<$bar>\n$template vars\n";
  StringFile sf(config1);
  cfg = new Config();
  assert (cfg != NULL);
  fprintf(stderr, "testing template single name/value pair with quotes....");
  assert(cfg->readFile(&sf) == 0);
  ConfigData *cd = cfg->root();
  assert(cd->type() == ConfigData::mdata);
  assert(cd->asMap().size() == 1);
  assert(cd->asMap().count(string("foobar1")) == 1);
  cd = (cd->asMap())[string("foobar1")];
  assert(cd != NULL);
  assert (cd->type() == ConfigData::sdata);
  assert(cd->asString() == string("baz1"));
  fprintf(stderr, "ok\n");
}

void ConfigTest::run_tests() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ConfigTest();
}
