/* SystemDirectory.cc -*- C++ -*-
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

#include "textus/file/SystemDirectory.h"
#include "textus/file/Directory.h"
#include "textus/file/FileHandle.h"

#include <sys/stat.h>

#include <string>

namespace textus { namespace file {
using namespace std;

class SystemDirectoryEntry: public DirectoryEntry {
private:
  string path;

  friend class SystemDirectory;

  explicit SystemDirectoryEntry(string p): path(p) {}

public:
  virtual ~SystemDirectoryEntry() {}

  FileHandle *open(int access, int mode=0600) {
    Synchronized(this);
    return FileHandle::openFile(path.c_str(), access, mode);
  }

  bool exists() {
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0) {
      return false;
    }
    return true;
  }
  
  bool unlink() {
    Synchronized(this);
    return ::unlink(path.c_str()) == 0;
  }

  string name() const {
    SynchronizedConst(this);
    size_t s;
    s = path.rfind('/');
    if (s == string::npos) {
      return path;
    }
    return path.substr(s+1);
  }

  bool isDir() const {
    SynchronizedConst(this);
    struct stat buf;
    int ret = stat(path.c_str(), &buf);
    if (ret != 0) {
      return false;
    }
    return buf.st_mode & S_IFDIR;
  }

  Time modified() const {
    SynchronizedConst(this);
    struct stat buf;
    int ret = stat(path.c_str(), &buf);
    if (ret != 0) {
      return Time((time_t)-1);
    }
    return Time(buf.st_mtime);
  }

  Directory *openDir() {
    Synchronized(this);
    if (!isDir()) {
      return NULL;
    }
    return new SystemDirectory(path);
  }
};

DirectoryEntry *SystemDirectory::current() {
  Synchronized(this);
  string p = pth + "/" + dent.d_name;
  return new SystemDirectoryEntry(p);
}

bool SystemDirectory::createPath(int mode) {
  for (string::size_type p = pth.find('/'); p != string::npos; p = pth.find('/', p+1)) {
    LOG(DEBUG) << "createPath: p=" << p << "\n";
    if (p == 0) {
      continue;
    }
    AUTODEREF(SystemDirectoryEntry *, sde);
    sde = new SystemDirectoryEntry(pth.substr(0, p));
    if (!sde->exists()) {
      LOG(INFO) << "Creating directory: " << pth.substr(0,p) << "\n";
      int ret = mkdir(pth.substr(0,p).c_str(), mode);
      if (ret != 0) {
	LOG(INFO) << "Unable to create directory " << pth.substr(0, p).c_str()
		  << " errno: " << errno << "\n";
	return false;
      }
    } else if (!sde->isDir()) {
      LOG(INFO) << "Tried to create directory over existing file: " << pth << "\n";
      return false;
    }
  }
  if (pth.length() == 0) {
    return true;
  }
  if (mkdir(pth.c_str(), mode) != 0) {
    LOG(INFO) << "Unable to create directory " << pth.c_str()
	       << " errno: " << errno << "\n";
    return false;
  }
  return true;
}

bool SystemDirectory::isDir() const {
    SynchronizedConst(this);
    struct stat buf;
    string p = pth + "/" + dent.d_name;
    int ret = stat(p.c_str(), &buf);
    if (ret != 0) {
      return false;
    }
    return buf.st_mode & S_IFDIR;
}

bool SystemDirectory::exists() {
    Synchronized(this);
    struct stat buf;
    string p = pth + "/" + dent.d_name;
    int ret = stat(p.c_str(), &buf);
    if (ret != 0) {
      return false;
    }
    return true;
}

Directory *SystemDirectory::newDirectory(string name, int mod) {
  int ret = 0;
  AUTODEREF(DirectoryEntry *, de);
  HRC(mkdir((pth + "/" + name).c_str(), mod));
  de = findEntry(name);
  HRNULL(de);
  HRTRUE(de->isDir());
  return de->openDir();
 error_out:
  return NULL;
}

}} //namespace
