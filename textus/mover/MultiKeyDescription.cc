/* MultiKeyDescription.cc -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
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

#include <set>

#include "textus/mover/MultiKeyDescription.h"

namespace textus { namespace mover {

bool MultiKeyDescription::process(string data) {
  int ret = 0;
  string out;
  string key_name;
  ReferenceValueMap<string, KeyDescription *>::iterator i = 
    descriptions.begin();
  HRFALSE(i == descriptions.end());
  HRC(encryption()->decrypt(i->second, data, &out, &key_name));
  
  i = descriptions.find(key_name);
  HRFALSE(i == descriptions.end());
  
  HRC(i->second->processMessage(data));
  
 error_out:
  return ret == 0;
}

bool MultiKeyDescription::addKeyDescription(KeyDescription *kd) {
  ReferenceValueMap<string, KeyDescription *>::iterator i = descriptions.begin();
  if (i == descriptions.end() || isCompatible(kd, i->second)) {
    descriptions[kd->at("key_name")] = kd;
    return true;
  }
  return false;
}

bool MultiKeyDescription::isCompatible(KeyDescription *k1, KeyDescription *k2) {
  int ret=0;
  set<string> keywords;
  HRNULL(encryption());
  keywords = encryption()->getDecryptionKeywords();
  for(set<string>::iterator i=keywords.begin(); i != keywords.end(); ++i) {
    HRTRUE(*i != "invalid_keyword");
    HRTRUE(k1->at(*i) == k2->at(*i));
  }
 error_out:
  return ret==0;
}


}} //namespace
