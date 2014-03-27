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
#include "textus/mover/KeyBook.h"
#include "textus/config/Config.h"
#include "textus/file/Shell.h"
#include "textus/template/Template.h"
#include "textus/system/Environment.h"
#include "textus/event/Message.h"

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

void KeyDescription::setVariables() {

  set("app-binary-path", textus::system::Environment::systemEnvironment().getVariable("APP_BINARY_PATH"));
  set("app-data-path", textus::system::Environment::systemEnvironment().getVariable("APP_PATH") + "/data");
  set("app-config-path", textus::system::Environment::systemEnvironment().getVariable("APP_PATH") + "/config");

}

int KeyDescription::processMessage(class KeyBook *kb, string message) {
  int ret=0;
  string cmd_template;
  string cmd;
  size_t offset;
  uint64_t magic;
  int type;

  AUTODEREF(Shell *, s);
  AUTODEREF(Template<KeyDescription *> *, t);

  HRTRUE (count("read") != 0);
  cmd_template = at("read");
  setVariables();
  //chomp the first bit.
  offset = message[0] & 7;
  HRTRUE (offset + 1 < message.length());
  message = message.substr(offset+1);

  HRTRUE(message.length() > 12); // make sure the header is there.
  magic = textus::event::Message::getUint64(message, 0);
  type = textus::event::Message::getUint32(message, 8);

  HRTRUE (magic == strtoull(attributes["magic"].c_str(), NULL, 0));

  message = message.substr(12);

  switch (type) {

  case MOVER_HEADER_TYPE_MESSAGE:
    /* Normal case, it's just a message. */
    break;

  case MOVER_HEADER_TYPE_RECURSIVE:
    HRTRUE(kb->processData(message));
    goto error_out;

  default:
    LOG(WARNING) << "Unknown message type: " << type << "\n";
    HRTRUE(type == MOVER_HEADER_TYPE_MESSAGE);
    break;
  }


  /* XXXXXXX FIXME: this code has been copied from External Encryption.
     it should only be in one place.
  */
  /* XXXXXXX Really fix me.  This code doesn't belong here at all, it belongs
     in ExternalEncryption.
  */
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
