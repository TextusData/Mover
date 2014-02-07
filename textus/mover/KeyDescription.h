/* KeyDescription.h -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
 *
 */

#include <string>
#include <map>

#include "textus/base/Base.h"
#include "textus/config/Config.h"

#ifndef TEXTUS_MOVER_KEY_DESCRIPTION_H_
#define TEXTUS_MOVER_KEY_DESCRIPTION_H_

namespace textus { namespace mover {

using namespace std;
using namespace textus::base;

class MoverEncryption;

class KeyDescription: virtual public Base {
private:
  map<string, string> attributes;
  bool temporary;
  AutoDeref<MoverEncryption> encryption;

protected:
public:
  static KeyDescription *fromConfigData(textus::config::ConfigData *cd);
  explicit KeyDescription(): temporary(false) {}
  virtual ~KeyDescription();

  void setTemporary(bool t=true) {
    temporary = t;
  }

  int addToConfigData(textus::config::ConfigData *cd);

  size_t count(string s) const { return attributes.count(s); }
  void set(string key, string value) { attributes[key] = value; }
  string at(string key) const { return attributes.at(key); }

  int processMessage(string message);
  
  MoverEncryption *getEncryption() {
    return encryption;
  }

  void setEncryption(MoverEncryption *e);
};

}} //namespace

#endif //TEXTUS_MOVER_KEY_DESCRIPTION_H_
