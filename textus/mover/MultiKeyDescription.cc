/* MultiKeyDescription.cc -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
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
