/* KeyDescription.cc -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
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

#include "textus/mover/KeyDescription.h"
#include "textus/mover/ExternalEncryption.h"
#include "textus/config/Config.h"
#include "textus/file/Shell.h"
#include "textus/template/Template.h"
#include "textus/system/Environment.h"

namespace textus { namespace mover {

using namespace textus::config;
using namespace textus::base;
using namespace textus::file;
using namespace textus::template_;
using namespace std;

KeyDescription::~KeyDescription() {
  if (temporary && encryption != NULL) {
    encryption->deleteKey(this);
  }
}

void KeyDescription::setEncryption(MoverEncryption *e) {
  encryption = e;
}

KeyDescription *KeyDescription::fromConfigData(ConfigData *cd) {
  KeyDescription *kd;
  kd = new KeyDescription();
  if (kd != NULL) {
    for (ConfigData::map_iterator i = cd->asMap().begin();
	 i != cd->asMap().end(); ++i) {
      if (i->second->type() == ConfigData::sdata) {
	kd->attributes[i->first] = i->second->asString();
      }
    }
  }
  return kd;
}

int KeyDescription::addToConfigData(ConfigData *cd) {
  int ret;
  if (temporary) {
    LOG(ERROR) << "Attempted to add temporary key to config data.\n";
    ret = -1;
    goto error_out;
  }
  HRTRUE(cd->type() == ConfigData::mdata);
  for (map<string, string>::iterator i = attributes.begin();
       i != attributes.end(); ++i) {
    if (cd->asMap().count(i->first) == 0) {
      AUTODEREF(ConfigData *, c);
      c = ConfigData::makeString(i->second);
      HRNULL(c);
      cd->asMap()[i->first] = c;
    }
  }
 error_out:
  return ret;
}

int KeyDescription::processMessage(string message) {
  int ret=0;
  string cmd_template;
  string cmd;
  AUTODEREF(Shell *, s);
  AUTODEREF(Template<KeyDescription *> *, t);

  HRTRUE (count("read") != 0);
  cmd_template = at("read");

  /* XXXXXXX FIXME: this code has been copied from External Encryption.
     it should only be in one place.
  */
  /* XXXXXXX Really fix me.  This code doesn't belong here at all, it belongs
     in ExternalEncryption.
  */
  set("app-binary-path", textus::system::Environment::systemEnvironment().getVariable("APP_BINARY_PATH"));
  t = new Template<KeyDescription *>(this);
  HRNULL(t);
  HRC(t->setBackingFile(mover_template_backing_file));
  HRC(t->process(cmd_template, &cmd));

  /* XXXXXX FIXME:  And this code is copied from ExternalEncryption
     encryt, which is the same as decrypt which is the same as ....
  */
  s = new Shell();
  HRNULL(s);

  s->setCommand(cmd);
  s->setInput(message);
  s->go();
  s->waitForCompletion();
  HRC(s->exitStatus());

 error_out:
  return ret;
  
  
}

}} //namespace
