/* AddressBookEntry.cc -*- c++ -*-
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

#include "textus/mover/AddressBookEntry.h"

namespace textus { namespace mover {

using namespace textus::base;
using namespace textus::file;
using namespace textus::config;
using namespace std;

string AddressBookEntry::encrypt(string data) {
  string out;
  int ret = 0;
  MoverEncryption *me = getCrypto();
  HRNULL(me);

  kd->setVariables();

  HRC(me->encrypt(kd, data, &out));
  data = "";

 error_out:
  if (ret == 0) {
    return out;
  }
  return "";
}

string AddressBookEntry::prepare(Mover *mover, string data) {
  int ret = 0;
  bool bottom=true;
  if (first.length() != 0) {
    AddressBookEntry *abe = parent->lookup(first);
    HRNULL(abe);
    data = abe->prepare(mover, data);
    HRZ(data.length());
    bottom = false;
  }
  if (header == "mt" || header.length() == 0) {
    data = mover->addHeader(magic, data, bottom ? MOVER_HEADER_TYPE_MESSAGE :
			    MOVER_HEADER_TYPE_RECURSIVE);
  } else {
    LOG(INFO) << "Unknown header type in send: " << header <<".\n";
  }

  data = encrypt(data);
 error_out:
  if (ret) {
    return string("");
  }
  return data;
}

int AddressBookEntry::send(Mover *mover, string data) {
  int ret = 0;
  Synchronized(this);
  data = prepare(mover, data);
  HRTRUE(data.length() > 12);
  HRC(mover->uploadData(data));

 error_out:
  return ret;
}

int AddressBookEntry::send(Mover *mover, TextusFile *tf) {
  string data = tf->read(mover_max_data_size);
  return send(mover, data);
}

MoverEncryption *AddressBookEntry::getCrypto() {
  Synchronized(this);
  if (crypto) {
    return crypto;
  }
  crypto = MoverEncryption::findEncryption(crypto_name);
  if (crypto == NULL) {
    LOG(WARNING) << "Unable to locate encryption: " << crypto_name << ".\n";
  }
  return crypto;
}

int AddressBookEntry::fromConfigData(ConfigData *cd) {
  int ret = 0;
  string tmp;
  ReferenceList<ConfigData *> *name_list;
  char *endptr;

  HRC(cd->getStringByName("crypto", &crypto_name));
  HRI(cd->getStringByName("first", &first));
  HRI(cd->getStringByName("header", &header));
  HRC(cd->getStringByName("magic", &tmp));
  magic = strtoull(tmp.c_str(), &endptr, 0);
  HRTRUE(*endptr == 0);
  kd = KeyDescription::fromConfigData(cd);
  HRNULL(kd);
  if (cd->getListByName("aliases", &name_list) == 0) {
    for (ReferenceList<ConfigData *>::iterator i = name_list->begin();
	 i != name_list->end(); ++i) {
      if ((*i)->asString().length() > 0) {
	names.push_back((*i)->asString());
      }
    }
  }
 error_out:
  return ret;
}

ConfigData *AddressBookEntry::asConfigData() {
  int ret = 0;
  AUTODEREF(ConfigData *, aliases);
  char buff[20];
  snprintf (buff, sizeof(buff), "%016llX", (long long unsigned) magic);
  string smagic(buff);
  ConfigData *cd = ConfigData::makeMap();
  HRNULL(cd);
  cd->asMap()["first"]  = ConfigData::makeString(first);
  cd->asMap()["crypto"] = ConfigData::makeString(crypto_name);
  cd->asMap()["magic"]  = ConfigData::makeString(smagic);
  cd->asMap()["header"] = ConfigData::makeString(header);

  aliases = ConfigData::makeList();
  HRNULL(aliases);
  cd->asMap()["alias"] = aliases;
  for (names_type::iterator i = names.begin(); i != names.end(); ++i) {
    AUTODEREF(ConfigData *, name);
    name = ConfigData::makeString(*i);
    HRNULL(name);
    aliases->asList().push_back(name);
  }

  HRC(kd->addToConfigData(cd));

 error_out:
  if (ret != 0) {
    if (cd) {
      delete cd;
      cd = NULL;
    }
  }
  return cd;
}

}} //namespace
