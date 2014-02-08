/* TemplateTest.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Test the template class.
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
#include "textus/template/Template.h"

using namespace textus::testing;
using namespace textus::template_;

namespace textus { namespace template_ {

class TemplateTest: public UnitTest {
private:

public:
  TemplateTest();
  ~TemplateTest();

  void run_tests();
};

TemplateTest::TemplateTest(): UnitTest(__FILE__)
{
}

TemplateTest::~TemplateTest()
{
}

void TemplateTest::run_tests() {
  map<string, string> vars;
  string bar("BAR");
  vars[string("FOO")]=bar;
  vars[string("FOZ")]="FOO";
  vars[string("FO2")]="FOZ";
  vars[string("BAZ")]="RAB";
  vars[string("RFOO")] = "<$FOO>";

  {
    fprintf(stderr, "Testing variable substitution....");
    string temp1("<$FOO>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == bar);
    fprintf(stderr, "ok\n");
  }

  {
    fprintf(stderr, "Testing conditional substitution...");
    string temp1("<?FOO:BAZ>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "BAZ");
    fprintf(stderr, "ok\n");
  }

  {
    fprintf(stderr, "Testing < inserstion ...");
    string temp1("<<This Text Is Ignored>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "<");
    fprintf(stderr, "ok\n");
  }

  {
    fprintf(stderr, "Testing > inserstion ...");
    string temp1("<>This Text Is Ignored>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == ">");
    fprintf(stderr, "ok\n");
  }
  
  {
    fprintf(stderr, "Testing comments ...");
    string temp1("<#This Text Is Ignored>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "");
    fprintf(stderr, "ok\n");
  }

  {
    fprintf(stderr, "Testing nested templates ...");
    string temp1("<?<$<$FO2>>:<$BAZ>>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "RAB");
    fprintf(stderr, "ok\n");
  }
  {
    fprintf(stderr, "Testing counters...");
    string temp1("<+A%05llu>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    tmpl.setBackingFile(getTmpFile());
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "A00000");
    temp2 = "";
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "A00001");
    fprintf(stderr, "ok\n");
    
  }
  {
    fprintf(stderr, "Testing recursive variable substitution....");
    string temp1("<$RFOO>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == bar);
    fprintf(stderr, "ok\n");
  }
  {
    fprintf(stderr, "Testing < substitution....");
    string temp1("<<>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == "<");
    fprintf(stderr, "ok\n");
  }
  {
    fprintf(stderr, "Testing > substitution....");
    string temp1("<>>");
    string temp2;
    Template< map<string, string> * > tmpl(&vars);
    assert(tmpl.process(temp1, &temp2) == 0);
    assert(temp2 == ">");
    fprintf(stderr, "ok\n");
  }

}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new TemplateTest();
}
