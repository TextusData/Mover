/* Config.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
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

#include "textus/config/Config.h"

namespace textus { namespace config {

static const string whitespace(" \t\r\n");
static const string word_start_chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>\\!@#$%^&*()./?'\";|");
static const string special_characters("\\=,\n\t\r ");

typedef int (Config::*command_func)(string, ConfigData *r);

static struct { string name; command_func f; } commands[]  = {
  {"include", &Config::include }
};

static string escapeSpecials(string in) {
  size_t p = 0;
  while ((p = in.substr(p).find_first_of(special_characters)) != string::npos) {
    in = in.substr(0,p-1) + "\\" + in.substr(p);
    p+=2;
  }
  return in;
}

ConfigData *ConfigData::makeMap() {
  ConfigData *cd = new ConfigData();
  if (cd != NULL) {
    cd->type_ = mdata;
  }
  return cd;
}

ConfigData *ConfigData::makeList() {
  ConfigData *cd = new ConfigData();
  if (cd != NULL) {
    cd->type_ = ldata;
  }
  return cd;
}

ConfigData *ConfigData::makeString(string value) {
  ConfigData *cd = new ConfigData();
  if (cd != NULL) {
    cd->type_ = sdata;
    cd->s_data = value;
  }
  return cd;
}

int ConfigData::writeFile(TextusFile *tf, int depth) {
  int ret = 0;

  switch (type()) {
  case sdata:
    {
      string escaped = escapeSpecials(asString());
      HRTRUE(tf->write(escaped) == (int)escaped.length());
      break;
    }

  case mdata:
    if (depth > 0) {
      HRTRUE(tf->write("{\n") == 2);
    }
    for (map_type::iterator i = asMap().begin();
	 i != asMap().end();
	 ++i) {
      string escaped = escapeSpecials(i->first);
      HRTRUE(tf->write(escaped + " = ") == (int)escaped.length() + 3);
      HRC(i->second->writeFile(tf, depth + 1));
      HRTRUE(tf->write(",\n") == 2);
    }
    if (depth > 0) {
      HRTRUE(tf->write("}\n") == 2);
    }
    break;

  case ConfigData::ldata:
    HRTRUE(tf->write("[") == 1);
    bool first = true;
    for (list_type::iterator i = asList().begin();
	 i != asList().end(); ++i) {
      if (!first) {
	HRTRUE(tf->write(", ") == 2);
      }
      (*i)->writeFile(tf, depth + 1);
      first = false;
    }
    HRTRUE(tf->write("]\n") == 2);
    break;
  }


 error_out:
  return ret;
}

int Config::writeFile(TextusFile *tf) {
  return root()->writeFile(tf);
}

int Config::writeFile(string name) {
  int ret = 0;
  AUTODEREF(TextusFile *, tf);
  tf = TextusFile::openFile(name, O_WRONLY);
  HRNULL(tf);
  HRC(writeFile(tf));
 error_out:
  return ret;
}

int Config::readFileSearchPath(const list<string> &paths, string name,
			       ConfigData *r) {
  int ret = 0;
  TextusFile *tf = TextusFile::openFileSearchPath(paths, name, O_RDONLY);
  HRNULL(tf);
  HRC(readFile(tf, r));
 error_out:
  return ret;
}

int Config::readFile(string name, ConfigData *r) {
  AUTODEREF(TextusFile *, tf);
  tf = TextusFile::openFile(name, O_RDONLY);
  if (tf == NULL) {
    return -1;
  }
  return readFile(tf, r);
}

int Config::handleCommand(string cmd, ConfigData *r) {
  for (unsigned i = 0; i < ARRAY_SIZE(commands); ++i) {
    if (cmd == commands[i].name) {
      return (this->*(commands[i].f))(cmd, r);
    }
  }
  return -1;
}

int Config::include(string cmd, ConfigData *r) {
  string file = cmd.substr(5);
  while (isspace(file[0])) {
    file = file.substr(1);
  }
  return readFile(file, r);
}

int Config::addValue(ConfigData *root, ConfigData *n) {
  switch(root->type()) {
  case ConfigData::mdata:
    (*root->m_data)[key] = n;
    key="";
    break;

  case ConfigData::ldata:
    root->l_data->push_back(n);
    break;

  case ConfigData::sdata:
    /* an error, a simple string should already have been handled. */
    return -1;
  }
  return 0;
}

int Config::readFile(TextusFile *fh, ConfigData *r) {
  int ret = 0;
  int linenum;
  int angle_bracket=0;
  size_t offset;
  size_t start_word = string::npos;
  string line;
  char c;
  bool saw_open_angle = false;
  if (r == NULL) {
    r = root();
  }
  if (r == NULL) {
    r = new ConfigData();
    HRNULL(r);
    r->type_ = ConfigData::mdata;
    setroot(r);
    r->m_data = new ConfigData::map_type();
    r->deref();
  }

  linenum = 0;
  for (TextusFile::iterator i = fh->begin(); i != fh->end(); ++i) {
    bool quote = false;
    offset = leftover.size();
    linenum++;
    line = leftover + *i;
    leftover = "";
    start_word = string::npos;
    for (size_t j = 0; j < line.size(); j++) {
      if (sawEscape || saw_open_angle) {
	sawEscape = false;
	saw_open_angle = false;
	continue;
      }
      if (start_word == string::npos &&
	  word_start_chars.find(line[j]) != string::npos) {
	start_word = j;
      }
      c = line[j];

      if (c == '"') {
	quote = !quote;
	line = line.substr(0, j) + line.substr(j+1);
	j = j - 1;
	continue;
      } else if (c == '\\') {
	sawEscape = true;
	line = line.substr(0, j) + line.substr(j+1);
	j = j - 1;
	continue;
      }


      if (quote) {
	continue;
      }

      if (angle_bracket) {
	if (c != '>' && c != '<') {
	  c  = 0;
	}
      }
      switch(c) {
      case '#':
	if (j > 0) {
	  line = line.substr(0, j-1);
	} else {
	  line = "";
	}
	break;

      case '$':
	HRC(handleCommand(line.substr(j), r));
	break;

      case '<':
	saw_open_angle = true;
	angle_bracket++;
	break;

      case '>':
	angle_bracket--;
	if (angle_bracket < 0) {
	  ret = -1;
	  goto error_out;
	}
	break;

      case '[':
	if (wantValue) {
	  AUTODEREF(ConfigData *, cd);
	  cd = new ConfigData();
	  wantValue = true;
	  cd->type_ = ConfigData::ldata;
	  cd->l_data = new ConfigData::list_type();
	  addValue(r, cd);
	  stack.push_front(r);
	  r = cd;
	  continue;
	}
	break;
      
      case '{':
	if (wantValue) {
	  AUTODEREF(ConfigData *, cd);
	  cd = new ConfigData();
	  wantValue = false;
	  cd->type_ = ConfigData::mdata;
	  cd->m_data = new ConfigData::map_type();
	  addValue(r, cd);
	  stack.push_front(r);
	  r = cd;
	  continue;
	}
	break;

      case ' ':
      case '\t':
	break;

      case ',':
      case ']':
      case '}':
	if (wantValue) {
	  AUTODEREF(ConfigData *, cd);
	  if (start_word == string::npos) {
	    LOG(INFO) << "parse error at line " << linenum << " character " << (j - offset) << ":\n";
	    LOG(INFO) << line.substr(j) << "\n";
	    ret = -1;
	    goto error_out;
	  }
	  string word = line.substr(start_word, j - start_word);
	  cd = new ConfigData();
	  HRNULL(cd);
	  cd->type_ = ConfigData::sdata;
	  cd->s_data = word;
	  addValue(r, cd);
	}

	start_word = string::npos;

	if (line[j] == ']') {
	  if (r->type_ != ConfigData::ldata) {
	    LOG(INFO) << "parse error at line " << linenum << " character " << (j - offset) << ":\n";
	    LOG(INFO) << line.substr(j) << "\n";
	    ret = -1;
	    goto error_out;
	  }
	  HRTRUE(stack.size() != 0);
	  r = stack.front();
	  stack.pop_front();
	} else if (line[j] == '}') {
	  if (r->type_ != ConfigData::mdata) {
	    LOG(INFO) << "parse error at line " << linenum << " character " << (j - offset) << ":\n";
	    LOG(INFO) << line.substr(j) << "\n";
	    ret = -1;
	    goto error_out;
	  }
	  HRTRUE(stack.size() != 0);
	  r = stack.front();
	  stack.pop_front();
	}

	switch (r->type_) {
	case ConfigData::ldata:
	  wantValue = true;
	  break;

	case ConfigData::mdata:
	  wantValue = false;
	  break;

	case ConfigData::sdata:
	    LOG(INFO) << "parse error at line " << linenum << " character " << (j - offset) << ":\n";
	    LOG(INFO) << line.substr(j) << "\n";
	  ret = -1;
	  goto error_out;
	}
	break;

      case '=':
	if (!wantValue) {
	  if (start_word == string::npos) {
	    LOG(INFO) << "parse error at line " << linenum << " character " << (j - offset) << ":\n";
	    LOG(INFO) << line.substr(j) << "\n";
	    ret = -1;
	    goto error_out;

	  }
	  AUTODEREF(ConfigData *, cd);
	  key = line.substr(start_word, j - start_word);
	  while (key.length() > 0 && isspace(key[key.length() - 1])) {
	    key = key.substr(0, key.length() - 1);
	  }
	  start_word = string::npos;
	  wantValue = true;
	  break;
	}

      default:
	break;

      }
    }
    if (wantValue) {
      AUTODEREF(ConfigData *, cd);
      if (start_word == string::npos) {
	LOG(INFO) << "parse error at end of line : line " << linenum << "\n";
	ret = -1;
	goto error_out;

      }
      string word = line.substr(start_word);
      start_word = string::npos;
      if (sawEscape) {
	leftover = word;
	sawEscape = false;
	continue;
      }
      cd = new ConfigData();
      HRNULL(cd);
      cd->type_ = ConfigData::sdata;
      cd->s_data = word;
      addValue(r, cd);
      wantValue = false;
    }
  }
  if (start_word != string::npos) {
    LOG(INFO) << "file ended in middle of parsing."
	      << line
	      << " left over.\n" ;
    ret = -1;
    goto error_out;
  }
  if (wantValue) {
    LOG(INFO) << "file ended in middle of parsing.\n";
    ret = -1;
    goto error_out;
  }
 error_out:
  if (ret != 0) {
    LOG(INFO) << "parse error at line " << linenum << "\n";
  }
  return ret;
}

}} //namespace
