/* AddressBook.cc -*- c++ -*-
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

#include "textus/mover/AddressBook.h"
#include "textus/base/init/Init.h"

namespace textus { namespace mover {

DEFINE_STRING_ARG(default_address_book_file_name, "address.cfg", "default_address_book_file_name", "The default address book.");

using namespace textus::base;
using namespace std;
using namespace textus::file;
using namespace textus::config;

static Base address_book_lock;

static AddressBook *default_book=NULL;

AddressBook *AddressBook::defaultBook() {
  Synchronized(&address_book_lock);
  if (default_book != NULL) {
    return default_book;
  }
  default_book = loadAddressBook(default_address_book_file_name);
  return default_book;
}

AddressBook *AddressBook::loadAddressBook(string file) {
  int ret = 0;
  AddressBook *ab = NULL;
  AUTODEREF(TextusFile *, fh);
  fh = TextusFile::openDataFile(file, O_RDONLY);
  HRNULL(fh);
  ab = loadAddressBook(fh);
  HRNULL(ab);
 error_out:
  if (ret != 0 && ab != NULL) {
    ab->deref();
  }
  return NULL;
}

AddressBook *AddressBook::loadAddressBook(TextusFile *fh) {
  int ret = 0;
  AUTODEREF(Config *, cfg);
  ConfigData *cd;
  AddressBook *addrbook = new AddressBook();

  cfg = new Config();
  HRC(cfg->readFile(fh));
  cd = cfg->root();
  HRNULL(cd);
  if (cd->type() != ConfigData::mdata) {
    LOG(INFO) << "Address book with root not a map.\n";
    HRC(-1);
  }

  for (ConfigData::map_iterator mi = cd->asMap().begin();
       mi != cd->asMap().end(); ++mi) {
    HRC(addrbook->addEntry(mi->first, mi->second));
  }

 error_out:
  if (ret < 0) {
    addrbook->deref();
    return NULL;
  }
  return addrbook;
}

int AddressBook::addEntry(string name, ConfigData *cd) {
  int ret = 0;
  AUTODEREF(AddressBookEntry *, abe);
  abe = new AddressBookEntry();
  HRNULL(abe);
  abe->setParent(this);
  abe->setName(name);
  HRC(abe->fromConfigData(cd));

  addEntry(abe);

 error_out:
  return ret;
}

AddressBookEntry *AddressBook::lookup(string name) {
  Synchronized(this);
  entries_iterator i = entries.find(name);
  if (i == entries.end()) {
    return NULL;
  }
  return i->second;
}

int AddressBook::addEntry(AddressBookEntry *abe) {
  Synchronized(this);
  AddressBookEntry::names_type nt = abe->getNames();
  for (AddressBookEntry::names_type::iterator i = nt.begin();
       i != nt.end(); ++i) {
    entries[*i] = abe;
  }
  return 0;
}

int AddressBook::delEntry(AddressBookEntry *abe) {
  Synchronized(this);
  AddressBookEntry::names_type nt = abe->getNames();
  for (AddressBookEntry::names_type::iterator i = nt.begin();
       i != nt.end(); ++i) {
    entries.erase(*i);
  }
  return 0;
}

int AddressBook::save(const char *file) {
  int ret=0;
  ConfigData *cd = NULL;
  AUTODEREF(Config*, cfg);
  AUTODEREF(ReferenceSet<AddressBookEntry *> *, abes);
  abes = entries.valueSet();
  HRNULL(abes);
  if (file == NULL) {
    file = file_name.c_str();
  }
  cfg = new Config();
  cd = cfg->root();
  
  HRNULL(cfg);
  for (ReferenceSet<AddressBookEntry *>::iterator i = abes->begin();
       i != abes->end(); ++i) {
    AUTODEREF(ConfigData *, cd2);
    cd2 = (*i)->asConfigData();
    HRNULL(cd2);
    cd->asMap()[(*i)->getName()] = cd2;
  }

  HRC(cfg->writeFile(file));

 error_out:
  return ret;
}


}} //namespace
