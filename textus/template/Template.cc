/* Template.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
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

#include "textus/template/Template.h"


namespace textus { namespace template_ {

using namespace std;
using namespace textus::base;

struct state {
  string *out;
  char cmd;
};

class KeywordCounter: virtual public Base {
private:
  set<string> keywords;
protected:
public:
  KeywordCounter() {}
  virtual ~KeywordCounter() {}

  int count(const string &key) {
    return 1;
  }

  string at(string key) {
    keywords.insert(key);
    return string ("");
  }

  set<string> getKeywordSet() {
    return keywords;
  }
};

set<string> TemplateBase::findKeyWords(string &temp) {
  int ret = 0;
  string out;
  AUTODEREF(Template<KeywordCounter *> *, t);
  AUTODEREF(KeywordCounter *, kc);
  kc = new KeywordCounter();
  HRNULL(kc);

  t = new Template<KeywordCounter *>(kc);
  HRNULL(t);

  HRC(t->process(temp, &out));
  return kc->getKeywordSet();

 error_out:
  set<string> err;
  err.insert(string("invalid_keyword"));
  return err;
}

int TemplateBase::evalNumTemplate(string in, string *out) {
  char buff[64];
  snprintf(buff, sizeof(buff), in.c_str(), getNextNum());
  return process(buff, out);
}

int TemplateBase::evalConditional(string in, string *out) {
  string value;
  string key;
  size_t p = in.find(':');
  if (p == string::npos) {
    return -1;
  }

  key = in.substr(0, p);
  int r = get(key, &value);
  if (r == 0 && value.size() > 0) {
    return process(in.substr(p+1), out);
  }
  return 0;
}

size_t TemplateBase::process(string in, string *out) {
  int ret = 0;
  size_t i;
  bool saw_open = false;
  list<state> stack;
  char cmd=-1;
  i = 0;
  while (i < in.length()) {
    string::value_type ch = in[i++];
    if (saw_open) {
      saw_open = false;
      cmd = ch;
      continue;
    }
    if (ch == '>') {
      state s;
      s = stack.front();
      stack.pop_front();
      switch (cmd) {
      default:
	// This is an unknown command error.
      case -1:
	// This is an extra > error.
	HRC(i);
	break;

      case '<':
	delete out;
	out = s.out;
	cmd = s.cmd;
	*out = *out + '<';
	break;

      case '>':
	delete out;
	out = s.out;
	cmd = s.cmd;
	*out = *out + '>';
	break;

      case '#':
	delete out;
	out = s.out;
	cmd = s.cmd;
	break;

      case '+':
	{
	  string value;
	  HRC(evalNumTemplate(*out, &value));
	  delete out;
	  out = s.out;
	  cmd = s.cmd;
	  *out = *out + value;
	  break;
	}


      case '$':
	{
	  string value;
	  HRC(get(*out, &value));
	  delete out;
	  out = s.out;
	  cmd = s.cmd;
	  *out = *out + value;
	  break;
	}

      case '?':
	{
	  string value;
	  HRC(evalConditional(*out, &value));
	  delete out;
	  out = s.out;
	  cmd = s.cmd;
	  *out = *out + value;
	  break;
	}
	  
      }
    } else if (ch != '<') {
      out->push_back(ch);
    } else {
      state s;
      s.out = out;
      s.cmd = cmd;
      saw_open=true;
      stack.push_front(s);
      cmd = -1;
      out = new string();
      HRNULL(out);
    }
  }
  HRC(stack.size());

  return 0;

 error_out:
  for (list<state>::iterator i = stack.begin(); i != stack.end(); ++i) {
    if (i->out != NULL) {
      delete i->out;
    }
  }
  return i;
}

}
} //namespace
