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
#include "textus/util/String.h"
#include "textus/util/STLExtensions.h"
#include "textus/template/Template.h"

namespace textus { namespace config {

static const string whitespace(" \t\r\n");
static const string word_start_chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>\\!@#$%^&*()./?'\";|");
static const string special_characters("\\=,\n\t\r ");


typedef int (Config::*command_func)(const list<string> &, ConfigData *r);

static struct { string name; command_func f; } commands[]  = {
  {"include", &Config::include },
  {"template", &Config::tmpl },
  {"eval", &Config::eval },
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



int ConfigData::walk(textus::base::functor::StringFunctor *sf, 
		     textus::base::functor::Functor *lf,
		     ConfigData::PairFunctor *pf) {
  int ret = 0;
  switch (type()) {
  case sdata:
    if (sf) {
      string out;
      HRC(sf->apply(s_data, &out));
      s_data = out;
    }
    break;
    
  case ldata:
    foreach (i, *l_data) {
      if (lf) {
	(*lf)(*i);
      }
      HRC((*i)->walk(sf, lf, pf));
    }
    break;

  case mdata:
    list< pair <string, string> > to_move;
    foreach (i, *m_data) {
      if (pf) {
	(*pf)(*i);
      }
      if (sf) {
	string out;
	HRC(sf->apply(i->first, &out));
	if (out != i->first) {
	  to_move.push_back(pair<string, string>(out, i->first));;
	}
      }
      HRC(i->second->walk(sf, lf, pf));
    }
    // Now we can change the keys.
    for(list< pair <string, string> >::iterator i = to_move.begin();
	i != to_move.end(); ++i) {
      (*m_data)[i->first] = (*m_data)[i->second];
      m_data->erase(i->second);
    }
    break;
  }

  if (lf) {
    /* Let the Functor know we are done
       with the current recursion. */
    (*lf)(NULL);
  }

  if (pf) {
    /* Let the Functor know we are done
       with the current recursion. */
    (*pf)(PairFunctor::pair(NULL, NULL));
  }

 error_out:
  return ret;

}

ConfigData *ConfigData::clone() {
  int ret = 0;
  ConfigData *cd=NULL; 
  switch (type()) {
  case sdata:
    cd = makeString(s_data);
    break;

  case ldata:
    cd = makeList();
    HRNULL(cd);
    for (list_iterator i = l_data->begin(); i != l_data->end(); ++i) {
      AUTODEREF(ConfigData *, cd2);
      cd2 = (*i)->clone();
      HRNULL(cd2);
      cd->l_data->push_back(cd2);
    }
    break;

  case mdata:
    cd = makeMap();
    HRNULL(cd);
    for (map_iterator i = m_data->begin(); i != m_data->end(); ++i) {
      AUTODEREF(ConfigData *, cd2);
      cd2 = i->second->clone();
      HRNULL(cd2);
      m_data->operator[](i->first) = cd2;
    }
    break;
  }

 error_out:
  if (ret != 0) {
    if (cd) {
      cd->deref();
    }
    return NULL;
  }
  return cd;
}


int ConfigData::applyTemplate(ConfigTemplate *t) {
  textus::template_::TemplateFunctor<UnionMap *> tf(t);
  return walk(&tf, NULL, NULL);
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
  int ret = 0;
  list<string> args = textus::util::StringUtils::splitAny(cmd, "\n\r\t ");
  string cmd_name;
  HRTRUE(args.size() > 0);
  cmd_name = args.front();
  // get rid of the leading $.
  cmd_name=cmd_name.substr(1);
  for (unsigned i = 0; i < ARRAY_SIZE(commands); ++i) {
    if (cmd_name == commands[i].name) {
      HRC((this->*(commands[i].f))(args, r));
      break;
    }
  }
 error_out:
  return ret;
}

int Config::include(const list<string> &args, ConfigData *r) {
  int ret = 0;
  int skip = 1;
  for (list<string>::const_iterator i = args.begin(); i != args.end(); ++i) {
    if (skip-- > 0) {
      continue;
    }
    string file = *i;
    if (file.length() > 0) {
      HRC(readFile(file, r));
    }
  }
 error_out:
  return ret;
}

int Config::tmpl(const list<string> &args, ConfigData *r) {
  int ret = 0;
  int skip=1;
  for(list<string>::const_iterator i = args.begin(); i != args.end(); ++i) {
    if (skip-- > 0) {
      continue;
    }
    string key = *i;
    if (key.length() > 0) {
      r->addTemplate(key);
    }
  }
  return ret;
}

int Config::eval(const list<string> &args, ConfigData *r) {
  int ret = 0;
  int count = 0;
  ConfigData::UnionMap map;
  ConfigData *cd = NULL;
  AUTODEREF(ConfigData::ConfigTemplate *, t);
  string templ;

  HRTRUE(args.size() > 2);
  HRTRUE(r->type() == ConfigData::mdata);

  t = new ConfigData::ConfigTemplate(&map);
  HRNULL(t);
  for (list<string>::const_iterator i = args.begin(); i != args.end(); ++i) {
    switch (count++) {
    case 0:
      continue;

    case 1:
      templ = *i;
      continue;
    }

    ConfigData::map_type *amap;
    if (i->size() == 0) {
      continue;
    }
    HRC(r->getMapByName(*i, &amap));
    map.addMap(amap);
  }

  HRTRUE(r->asMap().count(templ) > 0);
  cd = r->asMap()[templ];
  
  {
    AUTODEREF(ConfigData *, res);
    string newname;
    res = cd->clone();
    HRNULL(res);
    HRC(res->applyTemplate(t));
    HRC(t->process(templ, &newname));
    (r->asMap())[newname] = res;
  }

 error_out:
  return ret;
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
	  r->close();
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
	  r->close();
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
  r->close();
 error_out:
  if (ret != 0) {
    LOG(INFO) << "parse error at line " << linenum << "\n";
  }
  return ret;
}

}} //namespace
