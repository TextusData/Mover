/* KeyBook.cc -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
 *
 */

#include "textus/mover/KeyBook.h"
#include "textus/config/Config.h"

namespace textus { namespace mover {

using namespace textus::config;

int KeyBook::readFileSearchPath(const list<string> &paths, string name) {
  int ret = 0;
  AUTODEREF(Config *, cfg);
  ConfigData *cd;
  cfg = new Config();
  HRC(cfg->readFileSearchPath(paths, name));

  cd = cfg->root();
  HRNULL(cd);

  for (ConfigData::map_iterator i = cd->asMap().begin();
       i != cd->asMap().end(); ++i) {
    AUTODEREF(KeyDescription *, kd);
    bool handled=false;
    ConfigData *c = i->second;
    MoverEncryption *me;
    string name=i->first;
    string crypto;
    HRC(c->getStringByName("crypto", &crypto));
    me = MoverEncryption::findEncryption(crypto);
    if (me == NULL) {
      LOG(INFO) << "Unable to find encryption: " << crypto << 
	", ignoring key: " << name << ".\n";
      continue;
    }
    kd = KeyDescription::fromConfigData(c);
    kd->set(HUMAN_READABLE_NAME, name);
    foreach (i, descriptions) {
      if ((*i)->addKeyDescription(kd)) {
	handled = true;
	break;
      }
    }
    if (!handled) {
      AUTODEREF(MultiKeyDescription *, mkd);
      mkd = new MultiKeyDescription();
      HRNULL(mkd);
      mkd->set_encryption(me);
      HRTRUE(mkd->addKeyDescription(kd));
    }
  }
 error_out:
  return ret;
}

bool KeyBook::process(Mover *mover, string hash) {
  string data = mover->getDataForHash(hash);
  foreach(i, descriptions) {
    if ((*i)->process(data)) {
      return true;
    }
  }
  return false;
}

}} //namespace
