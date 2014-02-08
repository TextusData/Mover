/* Directory.h -*- C++ -*-
 * Copyright (C) 2010-2014 Ross Biro
 *
 * A class to process directories and their entries
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

#include "textus/base/Base.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/file/FileHandle.h"
#include "textus/base/predicate/Predicate.h"

#include <string>

#ifndef TEXTUS_FILE_DIRECTORY_H_
#define TEXTUS_FILE_DIRECTORY_H_

namespace textus { namespace file {

using namespace std;
using namespace textus::base;

class DirectoryEntry;
class DirectoryProvider;

class Directory: virtual public Base {
private:
  static ReferenceValueMap<string, DirectoryProvider *> providers;

protected:
  Directory() {}

public:

  typedef ReferenceList<DirectoryEntry *> Entries;

  // Use this instead of a constructor since we can have multiple providers.
  static Directory *lookup(string path);
  static void registerProvider(string nspace, DirectoryProvider *p);
  static void eraseProvider(string nspace);
  virtual ~Directory() {}

  virtual bool next() = 0;
  virtual bool eod() = 0;
  virtual bool previous() = 0;
  virtual bool rewind() = 0;
  virtual DirectoryEntry *current() = 0;
  virtual DirectoryEntry *findEntry(string name);
  virtual FileHandle *openCurrent(int acces, int mod=0600) = 0;
  virtual FileHandle *openFile(const char *name, int access, int mod=0600) = 0;
  virtual FileHandle *openFile(string name, int access, int mod=0600) = 0;
  virtual bool createPath(int mode=0755) = 0;
  virtual bool unlinkCurrent() = 0;
  virtual bool exists() = 0;
  virtual bool isDir() const = 0;
  virtual int symlink(string exsiting, string link) { return - 1; }

  virtual Entries *entries();
  virtual size_t entryCount();
  virtual bool recursiveUnlink();
  virtual FileHandle *open() { return NULL; }
  virtual Directory *newDirectory(string name, int mod=0755) = 0;
};

class DirectoryEntry: virtual public Base {
protected:
  DirectoryEntry() {}

public:
  virtual ~DirectoryEntry() {}  
  virtual FileHandle *open(int acces, int mode=0600) = 0;
  virtual bool unlink() = 0;
  virtual bool isDir() const = 0;
  virtual Directory *openDir() = 0;
  virtual string name() const = 0;
  virtual bool isFile() const { return !isDir(); }
  virtual Time modified() const = 0;
};

class IsDir: public textus::base::predicate::Predicate {
public:
  IsDir() {}
  virtual ~IsDir() {}
  virtual bool test(const Base *b) {
    const DirectoryEntry *de = dynamic_cast<const DirectoryEntry *>(b);
    return de != NULL && de->isDir();
  }
};

class IsFile: public textus::base::predicate::Predicate {
public:
  IsFile() {}
  virtual ~IsFile() {}
  virtual bool test(const Base *b) {
    const DirectoryEntry *de = dynamic_cast<const DirectoryEntry *>(b);
    return de != NULL && de->isFile();
  }
};

class NameMatches: public textus::base::predicate::Predicate {
private:
  string name_;

public:
  explicit NameMatches(string n): name_(n) {}
  virtual ~NameMatches() {}
  
  string name() { return name_;}
  void setname(string s) { name_ = s; }

  virtual bool test(const Base *b) {
    const DirectoryEntry *de = dynamic_cast<const DirectoryEntry *>(b);
    return de != NULL && de->name() == name();
  }
};

class DirectoryProvider: virtual public Base {
private:
public:
  DirectoryProvider() {}
  virtual ~DirectoryProvider() {}

  virtual Directory *provide(std::string path) = 0;
};

}} // namespace


#endif // TEXTUS_FILE_DIRECTORY_H_
