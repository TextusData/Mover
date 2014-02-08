/* Environmanet.h -*- C++ -*-
 * Copyright (C) 2010, 2013 Ross Biro
 *
 * A class process environment variables.
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

#ifndef TEXTUS_SYSTEM_ENVIRONMENT_H_
#define TEXTUS_SYSTEM_ENVIRONMENT_H_

#include <map>
#include <string>

#include "textus/base/Base.h"
#include "textus/base/init/Init.h"
#include "textus/base/AutoDeref.h"
#include "textus/file/TextusFile.h"


#define VARIABLE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"

namespace textus { namespace testing {
class UnitTest;
}} // namespace

namespace textus { namespace system {

using namespace std;

class Environment: virtual public textus::base::Base {
private:
  typedef textus::base::init::env_map env_map;
  typedef env_map::iterator iterator;
  typedef env_map::const_iterator const_iterator;

  env_map map;
  MVAR(public, Environment, parent)

  friend class textus::testing::UnitTest;
  static void modifySystemEnvironment(string key, string value);

public:
  static const Environment &systemEnvironment();
  static Environment &mutableSystemEnvironment();

  static int exportSystemEnvironment(string key, string value);

  static string getConfigPath(string file);
  static string getTmpPath(string file);
  static string getDataPath(string file) {
    string f = getConfigPath(file);
#ifdef TESTING
    if (textus::file::TextusFile::exists(f)) {
      return f;
    }
    return getTmpPath(file);
#else
    return f;
#endif
  }


  explicit Environment(env_map m) {
    for (iterator i = m.begin(); i != m.end(); ++i) {
      map[i->first] = i->second;
    }
  }

  Environment() {}
  virtual ~Environment() {}

  void setVariable(string key, string value) {
    Synchronized(this);
    map[key] = value;
  }

  Environment *clone() {
    Environment *e = new Environment();
    e->setparent(this);
    return e;
  }

  string getVariable(const string v) const {
    Synchronized(const_cast<Environment *>(this));
    const_iterator ci = map.find(v);
    if (ci == map.end()) {
      if (parent()) {
	return parent()->getVariable(v);
      }
      return string("");
    }
    return ci->second;
  }

  string getVariable(const char *c) const {
    return getVariable(string(c));
  }

  static string::size_type findMatchingParen(string s, string::size_type p) {
    int depth = 1;
    for (;p < s.length(); ++p) {
      if (s[p] == ')' && (p == 0 || s[p-1] != '\\')) {
	--depth;
	if (depth == 0) {
	  return p;
	}
      }
      if (s[p] != '$' || p+1 == s.length()) {
	continue;
      }
      if (s[p+1] == '$') {
	++p;
	continue;
      }
      if (s[p+1] == '(') {
	++depth;
	continue;
      }
    }

    return string::npos;
  }

  static string unescapeCloseParens(string f) {
    for (string::size_type s = 0; s < f.length()-1; s++) {
      if (f[s] != '\\') {
	continue;
      }
      if (f[s+1] == ')') {
	f = f.replace(s, 1, "");
      }
    }
    // XXXXX FIXME: valgrind reports this as a leak suppression:Unescape String Copy
    return f;
  }

  string expandStrings(string f) const {
    // $$->$, $FOO -> contents of FOO recursive, $(FOO) ditto
    bool parens = false;
    for (string::size_type s = 0; s < f.length(); s++) {
      if (f[s] != '$') {
	continue;
      }
      if (f[s+1] == '$') {
	f = f.replace(s, 1, "");
	continue;
      }
      string v;
      string::size_type l;
      if (f[s+1] == '(') {
	l = findMatchingParen(f, s+2);
	if (l != string::npos) {
	  ++l;
	  parens = true;
	}
      } else {
	l = f.find_first_not_of(VARIABLE_CHARS, s+1);
      }

      if (l == string::npos) {
	v = f.substr(s+1);
	f = f.replace(s, l, "");
      } else {
	v = f.substr(s+1, l-s-1);
	f = f.replace(s, l-s, "");
      }
      if (parens) {
	v = v.substr(1, v.length() - 2);
      }
      f = f.insert(s, getVariable(v));
    }

    f = unescapeCloseParens(f);
    return f;
  }
  
};

}} //namespace

#endif // TEXTUS_SYSTEM_ENVIRONMENT_H_
