/* MoverReaderMain.cc -*- c++ -*-
 * Copyright (C) 2014 Ross Biro
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

    string line;
    LineReaderStatus::status stat;

    ConfigData *cd=NULL;
    off_t pos;
    string value;
    list<string> search_path;
    mover = new Mover();
    HRNULL(mover);
    cf = new Config();
    HRNULL(cf);
    state_file = TextusFile::openWithPath(mover_user_root,
					  mover_reader_state_file,
					  O_RDWR);
    HRNULL(state_file);
    HRC(cf->readFile(state_file));
    cd = cf->root();
    HRC(cd->getStringByName("next_read", &value));
    pos = atoll(value.c_str());
    seen_file = TextusFile::openFile(mover_have_file_name, O_RDONLY);
    HRNULL(seen_file);
    HRTRUE(seen_file->seek(pos, SEEK_SET) == pos);

    search_path.push_back(mover_root);
    for (list<string>::iterator i = mover_key_book_dir.begin();
	 i != mover_key_book_dir.end(); ++i) {
      search_path.push_back(*i);
    }
    search_path.push_back(mover_user_root);

    kb = new KeyBook();
    HRNULL(kb);
    
    //Read in all the address books.
    for (list<string>::iterator i = mover_key_book_files.begin();
	 i != mover_key_book_files.end(); ++i) {
      HRC(kb->readFileSearchPath(search_path, *i));
    }
    
    lr = new LineReader(dynamic_cast<FileHandle *>(seen_file));
    for (line = lr->readLine(stat); stat == LineReaderStatus::ok;
	 line = lr->readLine(stat)) {
      while (line.length() > 0 &&isspace(line[line.length() - 1])) {
	line = line.substr(0, line.length() - 1);
      }
      if (line.length() > 0 && kb->process(mover, line)) {
	char buff[40];
	pos = state_file->seek(0, SEEK_CUR);
	snprintf(buff, sizeof(buff), "%ld", pos);
	cd->setStringByName(string("next_read"), string(buff));
	cf->writeFile(state_file);
	state_file->sync();
      }
    } 

    char buff[40];
    pos = state_file->seek(0, SEEK_CUR);
    snprintf(buff, sizeof(buff), "%ld", pos);
    cd->setStringByName("next_read", string(buff));
    cf->writeFile(state_file);
    state_file->sync();

  }
 error_out:
  TextusShutdown();
  return ret;
}
