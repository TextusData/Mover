/* AddressBookEntry.h -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
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
#ifndef TEXTUS_MOVER_ADDRESSBOOKENTRY_H_
#define TEXTUS_MOVER_ADDRESSBOOKENTRY_H_

#include  "textus/base/Base.h"
#include "textus/mover/AddressBook.h"
#include "textus/base/AutoDeref.h"
#include "textus/mover/MoverEncryption.h"
#include "textus/file/TextusFile.h"
#include "textus/config/Config.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"

namespace textus { namespace mover {

using namespace std;
using namespace textus::base;

class AddressBook;

class AddressBookEntry: public Base {
 public:
  typedef list<string> names_type;

 private:
  KeyDescription *kd;
  names_type names;
  AutoWeakDeref<AddressBook> parent;
  string name; // the cannonical name.
  AutoDeref<MoverEncryption> crypto;
  string crypto_name;
  string first;
  string header;
  uint64_t magic;

 protected:
  string prepare(Mover *mover, string data);
  string encrypt(string data);
  MoverEncryption *getCrypto();

 public:
  explicit AddressBookEntry() {}
  virtual ~AddressBookEntry() {}
  
  names_type &getNames() {
    return names;
  }

  string getName() {
    return name;
  }

  void setName(string n) {
    Synchronized(this);
    name = n;
    names.push_back(n);
  }

  int send(Mover *mover, string data);
  int send(Mover *mover, textus::file::TextusFile *tf);
  int fromConfigData(textus::config::ConfigData *);
  textus::config::ConfigData *asConfigData();

  void setParent(AddressBook *ab) {
    Synchronized(this);
    parent = ab;
  }
  
};

}} //namespace

#endif //TEXTUS_MOVER_ADDRESSBOOKENTRY_H_
