/* MoverSendMain.cc -*- c++ -*-
 * Copyright (C) 2013-2014 Ross Biro
 *
 */

#include "textus/mover/MoverEncryption.h"
#include "textus/mover/Mover.h"
#include "textus/mover/AddressBook.h"
#include "textus/base/init/Init.h"

using namespace textus::mover;
using namespace textus::base::init;

DEFINE_LIST_ARG(string, mover_destination_addresses, "destination_address", "a list of addresses to send messages to.", "");

int main(int argc, const char *argv[], const char *envp[])
{
  int ret = 0;
  TextusInit(argc, argv, envp);
  {
    AUTODEREF(Mover *, mover);
    ReferenceList<TextusFile *> files;
    ReferenceList<AddressBookEntry *> addresses;

    textus::random::PRNG::globalPRNG()->seed(time(NULL));

    mover = new Mover();
    HRNULL(mover);

    // validate all the destination addresses first.
    for (list<string>::iterator i = mover_destination_addresses.begin();
	 i != mover_destination_addresses.end();
	 ++i) {
      AUTODEREF(AddressBookEntry *, addr_book_entry);
      addr_book_entry = AddressBook::defaultBook()->lookup(*i);
      if (addr_book_entry == NULL) {
	LOG(INFO) << "Couldn't find address book entry for " << *i << ".\n";
	ret = 2;
	goto error_out;
      }
      addresses.push_back(addr_book_entry);
    }

    // check all the files.
    for (list<string *>::const_iterator i = getCommandLine().begin();
	 i != getCommandLine().end(); ++i) {
      AUTODEREF(TextusFile *, tf);
      tf = TextusFile::openFile(**i, O_RDONLY);
      if (tf == NULL) {
	LOG(INFO) << "Unable to open " << (*i)->c_str() << ".\n";
	ret = 1;
	goto error_out;
      }
      files.push_back(tf);
    }


    for (ReferenceList<TextusFile *>::iterator i = files.begin();
	 i != files.end();
	 ++i) {
      for (ReferenceList<AddressBookEntry *>::iterator j = addresses.begin();
	   j != addresses.end(); ++j) {
	HRC((*j)->send(mover, *i));
      }
    }
  }
 error_out:

  TextusShutdown();
  return ret;
}
