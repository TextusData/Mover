/* MoverEncryption.cc -*- c++ -*-
 * Coyright (c) 2013 Ross Biro 
 *
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

#include "textus/mover/MoverEncryption.h"
#include "textus/config/Config.h"
#include "textus/mover/ExternalEncryption.h"

namespace textus { namespace mover {

using namespace textus::config;

ReferenceValueMap<string, MoverEncryption *> MoverEncryption::encryptions;

MoverEncryption *MoverEncryption::findEncryption(string name) {
  if (encryptions.count(name) > 0) {
    return encryptions[name];
  }
  return NULL;
}

int MoverEncryption::registerEncryption(string name, MoverEncryption *me) {
  encryptions[name] = me;
  return 0;
}

int MoverEncryption::fromConfigFile(string name) {
  AUTODEREF(Config *, cfg);
  int ret = 0;
  ConfigData::map_iterator i;
  ConfigData *cd=NULL;

  cfg = new Config();
  HRNULL(cfg);
  HRC(cfg->readFile(name));

  cd = cfg->root();
  for (i = cd->asMap().begin(); i != cd->asMap().end(); ++i) {
    string name = i->first;
    LOG(INFO) << "Loading external encryption: " << name << ".\n";
    ConfigData *d = i->second;
    AUTODEREF(ExternalEncryption *, ee);
    ee = ExternalEncryption::fromConfig(name, d);
    HRNULL(ee);
    HRC(registerEncryption(name, ee));
  }
  
 error_out:
  return ret;
}

}} //mover
