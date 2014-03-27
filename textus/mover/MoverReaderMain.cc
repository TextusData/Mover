/* MoverReaderMain.cc -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
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

#include "textus/base/init/Init.h"
#include "textus/mover/MoverEncryption.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"
#include "textus/mover/AddressBook.h"
#include "textus/mover/KeyBook.h"
#include "textus/config/Config.h"
#include "textus/file/LineReader.h"


using namespace textus::mover;
using namespace textus::base::init;
using namespace textus::config;
using namespace textus::file;

DEFINE_LIST_ARG(string, mover_key_book_files, "mover_key_book", 
		"filename(s) where metadata about our keys", "keys.cfg");
DEFINE_STRING_ARG(mover_reader_state_file, "reader_state", "mover_reader_state_file_name", "The file used to keep track of what has been seen.");
DEFINE_LIST_ARG(string, mover_key_book_dir, "mover_key_book_dir", "directories to search for key book files.", "");

int main(int argc, const char *argv[], const char *envp[]) {
  int ret = 0;
  TextusInit(argc, argv, envp);
  {
    AUTODEREF(Mover *, mover);
    AUTODEREF(TextusFile *, state_file);
    AUTODEREF(TextusFile *, seen_file);
    AUTODEREF(KeyBook *, kb);
    AUTODEREF(Config *, cf);
    AUTODEREF(LineReader *, lr);
    AUTODEREF(Directory *, mroot);
    mroot = Directory::lookup(mover_user_root);
    HRNULL(mroot);
    if (!mroot->exists()) {
      HRTRUE(mroot->createPath(0700));
    }

    string line;
    ConfigData *cd=NULL;
    off_t pos;
    string value;
    list<string> search_path;
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

    HRC(mover->attachFileStore(textus::mover::mover_root));

    cf = new Config();
    HRNULL(cf);
    state_file = TextusFile::openWithPath(mover_user_root,
					  mover_reader_state_file,
					  O_RDWR|O_CREAT);
    HRNULL(state_file);
    HRC(cf->readFile(state_file));
    cd = cf->root();
    if (cd->getStringByName("next_read", &value)) {
      char buff[40];
      pos = 0;
      pos = state_file->seek(0, SEEK_CUR);
      snprintf(buff, sizeof(buff), "%ld", pos);
      cd->setStringByName("next_read", string(buff));
      // XXXXX FIXME.  Should write to a temp file and then
      // move it.
      state_file->seek(0);
      state_file->truncate(0);
      cf->writeFile(state_file);
      state_file->sync();
    } else {
      pos = atoll(value.c_str());
    }

    seen_file = TextusFile::openDataFile(mover_have_file_name, O_RDONLY);
    HRNULL(seen_file);
    HRTRUE(seen_file->seek(pos, SEEK_SET) == pos);

    search_path.push_front(mover_root);
    search_path.push_front(textus::base::init::getDataPath("."));
    for (list<string>::iterator i = mover_key_book_dir.begin();
	 i != mover_key_book_dir.end(); ++i) {
      search_path.push_front(*i);
    }
    search_path.push_front(mover_user_root);

    kb = new KeyBook();
    HRNULL(kb);
    
    //Read in all the address books.
    for (list<string>::iterator i = mover_key_book_files.begin();
	 i != mover_key_book_files.end(); ++i) {
      HRC(kb->readFileSearchPath(search_path, *i));
    }
    
    lr = new LineReader(dynamic_cast<FileHandle *>(seen_file));
    for (line = lr->waitForLine(); !lr->eof(); line = lr->waitForLine()) {
      while (line.length() > 0 &&isspace(line[line.length() - 1])) {
	line = line.substr(0, line.length() - 1);
      }
      if (line.length() > 0 && kb->process(mover, line)) {
	char buff[40];
	pos = state_file->seek(0, SEEK_CUR);
	snprintf(buff, sizeof(buff), "%ld", pos);
	cd->setStringByName(string("next_read"), string(buff));
	state_file->seek(0);
	state_file->truncate(0);
	//	cf->writeFile(state_file); XXXXXX Makes Debuggin Easier.
	state_file->sync();
      }
    } 

    char buff[40];
    pos = state_file->seek(0, SEEK_CUR);
    snprintf(buff, sizeof(buff), "%ld", pos);
    cd->setStringByName("next_read", string(buff));
    //cf->writeFile(state_file);
    state_file->sync();

  }
 error_out:
  TextusShutdown();
  return ret;
}
