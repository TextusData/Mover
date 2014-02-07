/* RegexpTest.cc -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
 *
 */
#include <list>
#include <string>

#include "textus/testing/UnitTest.h"
#include "textus/util/Regexp.h"

using namespace textus::testing;
using namespace textus::util;

namespace textus { namespace util {

class RegExpTest: public UnitTest {
private:

public:
  RegExpTest();
  ~RegExpTest();

  void run_tests();
};

RegExpTest::RegExpTest(): UnitTest(__FILE__)
{
}

RegExpTest::~RegExpTest()
{
}

/* args: regular express, success string, fail string */
bool simpleTest(string r, string s, string f, off_t start, off_t end) {
    RegExp<char> rt;
    ReferenceList<Match *> matches;
    
    fprintf(stderr, "compmiling regular expression /%s/....",
	    r.c_str());
    rt.compile(r);
    fprintf(stderr, "ok\n");
    fprintf(stderr, "testing regular expression /%s/ ...", r.c_str());
    assert(rt.match(s, &matches));
    assert (matches.size() == 1);
    assert (matches.front()->begin == start);
    assert (matches.front()->end == end);
    assert(!rt.match(f));
    fprintf(stderr, "ok\n");
    return true;
}

bool groupTest(string r, string s, list<string> groups) {
    RegExp<char> rt;
    ReferenceList<Match *> matches;
    Match *match;
    
    fprintf(stderr, "compmiling regular expression /%s/....",
	    r.c_str());
    rt.compile(r);
    fprintf(stderr, "ok\n");
    fprintf(stderr, "testing regular expression /%s/ ...", r.c_str());
    assert(rt.match(s, &matches));
    fprintf(stderr, "ok\n");
    fprintf(stderr, "Making sure group matches are correct....");
    assert (matches.size() == 1);
    match = matches.front();
    assert (match->group_matches != NULL);
    assert (groups.size() == match->ngroups);
    
    list<string>::iterator i = groups.begin();
    int j = 0;
    while (i != groups.end()) {
      off_t start;
      off_t end;
      if (match->group_matches[j].size() == 0) {
	start = 0;
	end = 0;
      } else {
	start = match->group_matches[j].front().first;
	end = match->group_matches[j].front().second;
      }
      if (*i != "") {
	assert(*i == s.substr(start, end - start));
      } else {
	assert (end == start);
      }
      ++i;
      ++j;
    }
    fprintf (stderr, "ok\n");
    return true;
}

void RegExpTest::run_tests() {
  simpleTest("abc", "cdefgabcefg", "abdefghilbggdcdebccbbcbca", 5,8);
  simpleTest("a+bc", "cdefgaabcefg", "abdefghilbggdcdebccbbcbca", 5, 9);
  simpleTest("a*bc", "cdefgbcefg", "abdefghilbggdcdeebdbabba", 5,7);
  simpleTest("ab|cd", "cdefgbcefg", "bbdefghilbggdceebdbcbbc", 0, 2);
  simpleTest("a(bc)d", "abcdefgbcefg", "abdefghilbggdcdeebdbabba",0,4);
  simpleTest("a(bc)*d", "abcbcbcdefgbcefg", "abdefghilbggdcdeebdbabba",0, 8);
  simpleTest("a(bc)+d", "abcbcbcdefgbcefg", "adbcdefghilbggdcdeebdbabba",
	     0, 8);
  simpleTest("a(b|c)+d", "abcbcbcdefgbcefg", "abcefghilbggdcdeebdbabba",
	     0, 8);
  simpleTest("(z|a|x)(b|c)+d", "abdefgbcefg", 
	     "adbcdefghilbggdcdeebdbabba", 0, 3);
  simpleTest("[zax](b|c)+d", "abcbcbcdefgbcefg", 
	     "adbcdefghilbggdcdeebdbabba", 0, 8);
  simpleTest("[zax](b|c)+d", "xbcbcbcdefgbcefg", 
	     "abcefghilbggdcdeebdbabba", 0, 8);
  simpleTest("\"([Teste ]* (A|B))", "\"Test Test A", "Test Test", 0, 12);
  simpleTest("\"([a-zA-Z0-9 ]*)\\(", "\"t(", "Test Test\\(", 0, 3);
  simpleTest("(a|a)b", "ab", "b", 0, 2);
  simpleTest("\"([a-zA-Z0-9 ]+)(\"|\\()", " gpg: encrypted with 2048-bit RSA key, ID 9878A3CF, created 2014-02-01\n\t \"Key4704\"", "foobar", 73, 82);

  list<string> groups;
  groups.push_back("bcbcbc");
  groupTest("a([bc]*)d", "bcbcbcbcabcbcbcdddd", groups);
  groups.clear();
  groups.push_back("d");
  groups.push_back("");
  groups.push_back("");
  groupTest("((a(bcd))*d)", "bcbcbcbcabcbcbcdddd", groups);
  groups.clear();
  groups.push_back("Key4704");
  groups.push_back("\"");
  groupTest("\"([a-zA-Z0-9 ]+)(\"|\\()",
	    " gpg: encrypted with 2048-bit RSA key, ID 9878A3CF, created 2014-02-01\n\t \"Key4704\"", groups);
  groups.clear();
  groups.push_back("");
  groupTest("a|(ab)c", "ac", groups);
}

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new RegExpTest();
}

