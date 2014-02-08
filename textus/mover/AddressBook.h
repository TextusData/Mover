/* AddressBook.h -*- c++ -*-
 * Copyright (C) 2013, 2014 Ross Biro
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

#ifndef TEXTUS_MOVER_ADDRESSBOOK_H_
#define TEXTUS_MOVER_ADDRESSBOOK_H_


#include "textus/base/Base.h"
#include "textus/mover/AddressBookEntry.h"
#include "textus/file/FileHandle.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/config/Config.h"

namespace textus { namespace mover {
class AddressBookEntry;

using namespace std;
using namespace textus::base;

class AddressBook: virtual public Base {
private:
  ReferenceValueMap<string, AddressBookEntry *> entries;
  typedef ReferenceValueMap<string, AddressBookEntry *>::iterator entries_iterator;

  string file_name;

protected:
  int addEntry(string name, textus::config::ConfigData *cd);

public:
  static AddressBook *defaultBook();
  static AddressBook *loadAddressBook(string file);
  static AddressBook *loadAddressBook(textus::file::TextusFile *fh);

  explicit AddressBook() {}
  virtual ~AddressBook() {}

  AddressBookEntry *lookup(string name);
  int addEntry(AddressBookEntry *abe);
  int save(const char *file=NULL);
  int delEntry(AddressBookEntry *abe);
  
};

}} //namespace

#endif //TEXTUS_MOVER_ADDRESSBOOK_H_
