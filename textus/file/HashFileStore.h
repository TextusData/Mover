/* HashFileStore.h -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
 *
 * A class to store files based on their hashes
 *
 */

#include "textus/file/Directory.h"
#include "textus/hash/Hasher.h"
#include "textus/file/FileBackedSet.h"
#include "textus/base/Time.h"

#include <string>
#include <set>

#ifndef TEXTUS_FILE_HASH_FILE_STORE_H_
#define TEXTUS_FILE_HASH_FILE_STORE_H_

namespace textus { namespace file {

using namespace std;

#define HEXCHARS "0123456789ABCDEF"

class HashFileStore: virtual public Base {
private:
  MVAR(protected, FileBackedSet<string>, hashes);
  MVAR(public, Directory, root);
  MVAR(public, textus::hash::Hasher, hashfunc);

public:
  HashFileStore() {}
  virtual ~HashFileStore() {}

  int openRoot(Directory *newroot, bool rebuild_directory=false);
  int assembleHashes(Directory *d, string prefix=string(""));
  Directory *findDirectoryFor(string, bool create = false);
  
  int expire(Directory *d, Time when, string path,  bool del=true);
  int expire(Time when, bool del=true);

  const set<string> availableHashes() const {
    return *hashes();
  }

  size_t maxlen() { return 1024 * 128; }
  string getData(string hash);
  int writeData(string data, bool overwrite);
  int writeData(string data, string hash, bool overwrite);
  int newData(string data) { return writeData(data, false);}

  bool contains(string s) const {
    return (hashes()->count(s) > 0);
  }

  bool seen(string s);

  size_t size() const {
    return hashes()->size();
  }
};

}} //namespace

#endif // TEXTUS_FILE_HASH_FILE_STORE_H_
