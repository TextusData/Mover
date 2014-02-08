/* KeyDescription.h -*- c++ -*-
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
