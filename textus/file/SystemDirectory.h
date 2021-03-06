/* SystemDirectory.h -*- C++ -*-
 * Copyright (C) 2010, 2013, 2014 Ross Biro
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

#ifndef TEXTUS_FILE_SYSTEM_DIRECTORY_H_
#define TEXTUS_FILE_SYSTEM_DIRECTORY_H_
#include "textus/file/Directory.h"

#include <dirent.h>

namespace textus { namespace file {

class SystemDirectory: public Directory {
private:
  DIR *dr;
  struct dirent dent;
  string pth;
  bool done;

public:
  SystemDirectory(string p): dr(NULL), pth(p), done(true) {
    if (pth[0] != '/') {
      char buff[PATH_MAX+2];
      char *ptr = realpath (".", buff);
      if (ptr == NULL)  {
	LOG(ERROR) << "Can't resolve .\n";
	die();
      }
      pth = string(ptr) + "/" + pth;
    }
    dr = opendir(pth.c_str());
    if (dr) {
      done = false;
      next();
    }
  }

  virtual ~SystemDirectory() {
    if (dr) {
      closedir(dr);
      dr = NULL;
    }
  }

  virtual bool eod() {
    Synchronized(this);
    return done;
  }

  virtual bool next() {
    struct dirent *d2;
    Synchronized(this);
    int res = readdir_r(dr, &dent, &d2);
    if (d2 == NULL || res != 0) {
      if (res != 0) {
	LOG(INFO) << "SystemDirectory::next error " << errno << "\n";
      }
      done = true;
      return false;
    }
    return true;
  }

  virtual string pathName() {
    return pth;
  }

  virtual bool rewind() {
    Synchronized(this);
    done = false;
    rewinddir(dr);
    return next();
  }

  virtual bool previous() {
    Synchronized(this);
    done = false;
    long pos = telldir(dr);
    seekdir(dr, pos - 1);
    return next();
  }

  virtual FileHandle *openCurrent(int access, int mod=0600) {
    Synchronized(this);
    string p = pth + "/" + dent.d_name;
    return FileHandle::openFile(p.c_str(), access, mod);
  }

  virtual FileHandle *openFile(string name, int access, int mod=0600) {
    Synchronized(this);
    string p = pth + "/" + name;
    return FileHandle::openFile(p.c_str(), access, mod);
  }

  virtual int symlink(string name, string to) {
    Synchronized(this);
    string p = pth + "/" + name;
    unlink(p.c_str());
    return symlink(to.c_str(), p.c_str());
  }

  virtual FileHandle *openFile(const char *name, int access, int mod=0600) {
    Synchronized(this);
    string p = pth + "/" + name;
    return FileHandle::openFile(p.c_str(), access, mod);
  }


  virtual DirectoryEntry *current();
  virtual bool createPath(int mode=0755);
  virtual bool exists();
  virtual bool isDir() const;
  virtual bool unlinkCurrent() {
    Synchronized(this);
    string p = pth + "/" + dent.d_name;
    return unlink(p.c_str()) == 0;
  }

  virtual Directory *newDirectory(string name, int mod=0600);

};

}} //namespace

#endif // TEXTUS_FILE_SYSTEM_DIRECTORY_H_
