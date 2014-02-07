/* Config.h -*- c++ -*-
 * Copyright (C) 2013, 2014 Ross Biro
 *
 * Read config files.
 */

#ifndef TEXTUS_CONFIG_CONFIG_H_
#define TEXTUS_CONFIG_CONFIG_H_

#include "textus/base/Base.h"
#include "textus/file/FileHandle.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/ReferenceValueMap.h"

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

private:
  friend class Config;
  Type type_;
  string key;
  AutoDeref<list_type> l_data;
  AutoDeref<map_type> m_data;
  string s_data;

protected:
  explicit ConfigData() {}

public:
  static ConfigData *makeMap();
  static ConfigData *makeList();
  static ConfigData *makeString(string v);
  virtual ~ConfigData() {}

  int writeFile(TextusFile *tf, int depth=0);
  Type type() const { return type_; }
  string asString() { return s_data; }
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

  int include(string, ConfigData *);
  int handleCommand(string, ConfigData *);
  int addValue(ConfigData *root, ConfigData *n);

  int writeFile(string file);
  int writeFile(TextusFile *fh);
  
};

}} //namespace



#endif //TEXTUS_CONFIG_CONFIG_H_
