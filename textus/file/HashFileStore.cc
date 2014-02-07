/* HashFileStore.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
 *
 * A class to store files based on their hashes
 *
 */

#include <errno.h>

#include "textus/file/HashFileStore.h"
#include "textus/util/Hex.h"

namespace textus { namespace file {

Directory *HashFileStore::findDirectoryFor(string h, bool create) {
  Synchronized(this);
  Directory *d = root();
  int ret = 0;
  d->ref();
  while (h.length() > 2) {
    DirectoryEntry *de;
    de = d->findEntry(h.substr(0,2));
    if (de == NULL) {
      if (!create) {
	goto error_out;
      }
      Directory *d2;
      d2 = d->newDirectory(h.substr(0,2));
      HRNULL(d2);
      d -> deref();
      d = d2;
    } else {
      HRTRUE(de->isDir());
      d->deref();
      d = de->openDir();
      de->deref();
    }
    h = h.substr(2);
  }
  return d;
 error_out:
  return NULL;
}

class NameHex2: public textus::base::predicate::Predicate {
private:

public:
  explicit NameHex2() {}
  virtual ~NameHex2() {}
  
  virtual bool test(const Base *b) {
    const DirectoryEntry *de = dynamic_cast<const DirectoryEntry *>(b);
    return de != NULL && de->name().length() == 2 &&
      de->name().find_first_not_of(HEXCHARS) == string::npos;
  }
};

int HashFileStore::assembleHashes(Directory *d, string prefix) {
  int ret = 0;
  textus::base::predicate::Filters<Directory::Entries> f;
  AUTODEREF(Directory::Entries *, e);
  NameHex2 n;
  e = f.filterMatching(d->entries(), &n);
  for (Directory::Entries::iterator i = e->begin(); i != e->end(); ++i) {
    DirectoryEntry *de = *i;
    if (de->isDir()) {
      HRC(assembleHashes(de->openDir(), prefix+de->name()));
    } else {
      hashes()->insert(prefix+de->name());
    }
  }
 error_out:
  return ret;
}

int HashFileStore::openRoot(Directory *rd, bool rebuild_hash) {
  AUTODEREF(FileHandle *, fh);
  AUTODEREF(DirectoryEntry *,kh);
  int ret = 0;
  setroot(rd);
  sethashes(new FileBackedSet<string>);
  HRNULL(hashes());
  hashes()->deref();
  kh = rd->findEntry("known_hashes");
  fh = rd->openFile("known_hashes", O_RDWR|O_CREAT);
  HRNULL(fh);
  hashes()->setBackingFile(fh);
  if (kh == NULL || rebuild_hash) {
    hashes()->clear();
    HRC(assembleHashes(rd, ""));
  }
  
  return 0;
 error_out:
  sethashes(NULL);
  return -1;
}

string HashFileStore::getData(string hash) {
  AUTODEREF(FileHandle *, fh);
  AUTODEREF(Directory *, d);
  int ret = 0;
  string str;
  HRNULL(d = findDirectoryFor(hash));
  hash = hash.substr(hash.length()-2);
  fh = d->openFile(hash, O_RDONLY);
  HRNULL(fh);
  str = fh->read(maxlen());
 error_out:
  if (!ret) {
    return str;
  } else {
    return string("");
  }
}

int HashFileStore::writeData(string data, bool overwrite) {
  string hash = hashfunc()->computeHash(data);
  hash = textus::util::hex_encode(hash);

  return writeData(data, hash, overwrite);
}

int HashFileStore::expire(Directory *d, Time when, string path, bool del) {
  int ret = 0;
  textus::base::predicate::Filters<Directory::Entries> f;
  AUTODEREF(Directory::Entries *, e);
  NameHex2 n;
  e = f.filterMatching(d->entries(), &n);
  for (Directory::Entries::iterator i = e->begin(); i != e->end(); ++i) {
    DirectoryEntry *de = *i;
    if (de->isDir()) {
      AUTODEREF(Directory *, child);
      child = de->openDir();
      ret = ret + expire(child, when, path + de->name(), del);
      if (child->entryCount() == 0) {
	de->unlink();
      }
      continue;
    }
    if (de->modified() < when) {
      if (del) {
	de->unlink();
      } else {
	d->symlink(de->name(), string("/dev/null"));
      }
      hashes()->removeElement(path + de->name());
      ret ++;
    }
  }
  return ret;
}

int HashFileStore::expire(Time when, bool del) {
  return expire(root(), when, string(""), del);
}

// Hack.  If it's a symlink to dev-null, it's been deleted, but we
// have seen it before.
bool HashFileStore::seen(string s) {
  if (contains(s)) {
    return true;
  }
  AUTODEREF(Directory *, d);
  d = findDirectoryFor(s, false);
  if (d == NULL) {
    return false;
  }
  if (d->findEntry(s.substr(s.length() - 2))) {
    return true;
  }
  return false;
}


int HashFileStore::writeData(string data, string hash, bool overwrite) {
  int ret = 0;
  AUTODEREF(Directory *, d);
  AUTODEREF(FileHandle *, fh);

  d = findDirectoryFor(hash, true);
  if (d != NULL) {
    Synchronized(d);
  }
  HRNULL(d);
  if (!overwrite && d->findEntry(hash.substr(hash.length()-2))) {
    ret = -EEXIST;
    goto error_out;
  }

  fh = d->openFile(hash.substr(hash.length()-2), O_RDWR|O_CREAT);
  HRNULL(fh);
  
  if (fh->write(data) != (long)data.length()) {
    ret = -1;
    goto error_out;
  }

  hashes()->insert(hash);

 error_out:
  if (fh) {
    fh->close();
  }

  return ret;

}


}} //namespace
