/* Config.h -*- c++ -*-
 * Copyright (C) 2013, 2014 Ross Biro
 *
 * Read config files.
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

#ifndef TEXTUS_CONFIG_CONFIG_H_
#define TEXTUS_CONFIG_CONFIG_H_

#include "textus/base/Base.h"
#include "textus/file/FileHandle.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/base/UnionReferenceMap.h"
#include "textus/template/Template.h"

namespace textus { namespace config {
using namespace std;
using namespace textus::base;
using namespace textus::file;

class ConfigData: virtual public Base {
public:
  enum Type {
    sdata,
    ldata,
    mdata
  };

  typedef ReferenceValueMap<string, ConfigData *> map_type;
  typedef map_type::iterator map_iterator;
  typedef ReferenceList<ConfigData *> list_type;
  typedef list_type::iterator list_iterator;

  class UnionMap: virtual public Base {
  private:
    UnionReferenceMap<ConfigData::map_type, ConfigData *> urm;

  public:
    explicit UnionMap() {}
    virtual ~UnionMap() {}

    void addMap(map_type *m) {
      urm.addMap(m);
    }
    
    size_t count (string k) {
      return urm.count(k);
    }

    bool contains (string k) {
      return urm.contains(k);
    }

    string at(string key) {
      ConfigData *cd = urm.at(key);
      return cd->asString();
    }
  };
  
  typedef textus::template_::Template<UnionMap *> ConfigTemplate;
  typedef textus::base::functor::PairFunctor<string,const ConfigData *> PairFunctor;


private:
  friend class Config;
  Type type_;
  //string key; -- is this really used?  Delete if not.
  AutoDeref<list_type> l_data;
  AutoDeref<map_type> m_data;
  string s_data;

  ConfigData *clone();
  list<string> template_keys;

protected:
  explicit ConfigData() {}

public:
  static ConfigData *makeMap();
  static ConfigData *makeList();
  static ConfigData *makeString(string v);
  virtual ~ConfigData() {}

  void addTemplate(string key) {
    // save them to delete later.
    template_keys.push_back(key);
  }

  void close() {
    if (type() == mdata) {
      for (list<string>::iterator i = template_keys.begin();
	   i != template_keys.end(); ++i) {
	m_data->erase(*i);
      }
    }
  }

  int writeFile(TextusFile *tf, int depth=0);
  Type type() const { return type_; }
  string asString() const { return s_data; }
  ReferenceList<ConfigData *> &asList() { return *l_data; }
  ReferenceValueMap<string, ConfigData *> &asMap() { return *m_data; }

  int getStringByName(string key, string *value) {
    int ret = 0;
    ConfigData *c;
    HRTRUE(asMap().count(key) > 0);
    c = asMap()[(key)];
    HRTRUE(c->type() == ConfigData::sdata);
    *value = c->asString();
  error_out:
    return ret;
  }

  int setStringByName(string key, string value) {
    int ret = 0;
    asMap()[(key)] = makeString(value);
    return ret;
  }

  int getMapByName(string key,
		   ReferenceValueMap<string,
		       textus::config::ConfigData *> **value) {
    int ret = 0;
    ConfigData *c;
    HRTRUE(asMap().count(key) > 0);
    c = asMap()[(key)];
    HRTRUE(c->type() == ConfigData::mdata);
    *value = &c->asMap();
  error_out:
    return ret;
  }

  int getListByName(string key,
		    ReferenceList<textus::config::ConfigData *> **value) {
    int ret = 0;
    ConfigData *c;
    HRTRUE(asMap().count(key) > 0);
    c = asMap()[(key)];
    HRTRUE(c->type() == ConfigData::ldata);
    *value = &c->asList();
  error_out:
    return ret;
  }

  int applyTemplate(ConfigTemplate *t);
  int walk(textus::base::functor::StringFunctor *sf,
	   textus::base::functor::Functor *lf,
	   PairFunctor *mf);

};

class Config: virtual public Base {
private:
  MVAR(public, ConfigData, root);

protected:
  bool wantValue;
  bool sawEscape;
  ReferenceList<ConfigData *> stack;
  string leftover;
  string key;
  
public:
  Config(): wantValue(false), sawEscape(false)  {}
  virtual ~Config() {}

  int readFileSearchPath(const list<string> &paths, string name,
			 ConfigData *r = NULL);
  int readFile(string name, ConfigData *r = NULL);
  int readFile(TextusFile *fh, ConfigData *r = NULL);

  int include(const list<string> &, ConfigData *);
  int tmpl(const list<string> &, ConfigData *);
  int eval(const list<string> &, ConfigData *);
  int handleCommand(string, ConfigData *);
  int addValue(ConfigData *root, ConfigData *n);

  int writeFile(string file);
  int writeFile(TextusFile *fh);
  
};

}} //namespace



#endif //TEXTUS_CONFIG_CONFIG_H_
