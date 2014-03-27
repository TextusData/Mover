/* MoverSendMain.cc -*- c++ -*-
 * Copyright (C) 2013-2014 Ross Biro
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
#include "textus/mover/Mover.h"
#include "textus/mover/AddressBook.h"
#include "textus/base/init/Init.h"

using namespace textus::mover;
using namespace textus::base::init;

DEFINE_LIST_ARG(string, mover_destination_addresses, "destination_address", "a list of addresses to send messages to.", "");
DEFINE_BOOL_ARG(mover_send_stdin, false, "send_stdin", "Send the contents of stdin.");

int main(int argc, const char *argv[], const char *envp[])
{
  int ret = 0;
  TextusInit(argc, argv, envp);
  {
    AUTODEREF(Mover *, mover);
    AUTODEREF(MoverVerifier *, mv);
    ReferenceList<TextusFile *> files;
    ReferenceList<AddressBookEntry *> addresses;

    textus::random::PRNG::globalPRNG()->seed(time(NULL));

    mover = new Mover();
    HRNULL(mover);
    string config_path = mover_crypto_config;
    TextusFile *tf = TextusFile::openConfigFile(config_path, O_RDONLY);
    if (tf == NULL) {
      fprintf (stderr, "Unable to open crypto.cfg: %s\n", config_path.c_str());
      exit (3);
    }

    if (MoverEncryption::fromConfigFile(tf) != 0) {
      fprintf (stderr, "Unable to read crypto.cfg file: %s\n",
	       config_path.c_str());
      exit (2);
    }


    mover->bindCertificate(mover_certificate);
    mv = new MoverVerifier();
    mover->setverifier(mv);

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
    if (mover_send_stdin) {
      AUTODEREF(TextusFile *, tf);
      tf = TextusFile::openStdin();
      HRNULL(tf);
      files.push_back(tf);
    } 

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
