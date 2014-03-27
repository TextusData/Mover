/* SystemFile.cc -*- c++ -*-
 * Copyright (c) 2009, 2014=2014 Ross Biro
 *
 * Represents a system file.
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

#include <sys/stat.h>

#include "textus/file/TextusFile.h"
#include "textus/file/SystemFile.h"
#include "textus/file/FileHandle.h"
#include "textus/base/init/Init.h"
#include "textus/util/STLExtensions.h"
#include "textus/util/String.h"
#include "textus/system/SysInfo.h"

#define MAX_ITERATOR_READ 16384

namespace textus { namespace file {

string TextusFile::pathJoin(const string &p1, const string &p2) {
  if (p1.length() == 0) {
    return FILE_SEP_STRING + p2;
  }
  if (p1[p1.length() - 1] == FILE_SEP_CHAR) {
    return p1 + p2;
  }
  return p1 + FILE_SEP_CSTR + p2;
}

list<string> TextusFile::pathSplit(const string &p1) {
  return textus::util::StringUtils::split(p1, FILE_SEP_CHAR);
}

bool TextusFile::absPath(const string &p) {
  return p[0] == FILE_SEP_CHAR;
}

string TextusFile::expandPath(const string &p1) {
  string p2;
  list<string> components = pathSplit(p1);
  foreacht(list<string>, c, components) {
    switch((*c)[0]) {
    case '~': 
      {
	string user;
	user = c->substr(1);
	string home_dir = 
	  textus::system::SysInfo::systemInformation()->getHomeDir(user);
	p2 = pathJoin(p2, home_dir);
	break;
      }

    case '\\':
      {
	p2 = pathJoin(p2, c->substr(1));
	break;
      }

    default:
      p2 = pathJoin(p2, *c);
      break;
    }
  }
  return p2;
}

TextusFile *TextusFile::openStdin() {
  FileHandle *tf = new FileHandle();
  if (tf != NULL) {
    tf->open(0);
  }
  return static_cast<TextusFile *>(tf);
}

TextusFile *TextusFile::openWithPath(const string path, const string name,
				     int acces, int md) {
  if (absPath(name)) {
    return openFile(name, acces, md);
  }
  return openFile(pathJoin(path, name), acces, md);
}

TextusFile *TextusFile::openConfigFile(const string name, int acess, int md)
{
  if (absPath(name)) {
    return openFile(name, acess, md);
  }
  return openFile(textus::base::init::getConfigPath(name), acess, md);
}

TextusFile *TextusFile::openDataFile(const string name, int acess, int md) {
  if (absPath(name)) {
    return openFile(name, acess, md);
  }
  return openFile(textus::base::init::getDataPath(name), acess, md);
}

TextusFile *TextusFile::openFileSearchPath(const list<string> paths,
					   const string name, int acess,
					   int md) {
  int ret = 0;
  TextusFile *tf;
  if (name[0] == FILE_SEP_CHAR) {
    tf = openFile(name, acess, md);
    goto error_out;
  }

  for (list<string>::const_iterator i = paths.begin();
       i != paths.end(); ++i) {
    tf = openWithPath(*i, name, acess, md);
    if (tf != NULL) {
      goto error_out;
    }
  }


 error_out:
  if (ret < 0) {
    if (tf) {
      tf->deref();
    }
    return NULL;
  }
  return tf;
}

string TextusFile::readAll(int len) {
  string r;
  while (len > 0) {
    string s = read(len);
    if (s == "") {
      break;
    }
    r = r + s;
    len = len - s.length();
  }
  return r;
}

TextusFile *TextusFile::openFile(const string name, int access, int mode)
{
  SystemFile *sf = new SystemFile();
  if (sf->open(name, access, mode) < 0) {
    delete sf;
    return NULL;
  }

  return sf;
}

bool TextusFile::exists(string name) {
  struct stat stat_buf;
  return stat(name.c_str(), &stat_buf) == 0;
}

int TextusFile::iterator::readMore() {
  if (parent == NULL) {
    LOG(DEBUG) << "parent null in iterator::readMore" << std::endl;
    return -1;
  }
  Synchronized(parent);
  parent->seek(off);
  buff_base = off;
  string b = parent->read(MAX_ITERATOR_READ);
  if (b.length() == 0) {
    return -1;
  }
  off = parent->tell();
  buff = buff + b;
  return 0;
}

TextusFile::iterator::iterator(const TextusFile::iterator &old) {
  parent = old.parent;
  off = old.off;
  buff = old.buff;
  buff_base = old.buff_base;
}

string TextusFile::iterator::operator *() {
  size_t s = buff.find('\n');
  while (s == string::npos) {
    if (readMore()) {
      break;
    }
    s = buff.find('\n');
  }
  if (s == string::npos) {
    string r = buff;
    buff = string();
    return r;
  }
  string r = buff.substr(0, s);
  return r;
}

TextusFile::iterator &TextusFile::iterator::operator++() {
  size_t s = buff.find('\n');
  while (s == string::npos) {
    if (readMore()) {
      break;
    }
    s = buff.find('\n');
  }
  if (s == string::npos) {
    buff = string();
    off = -1;
    buff_base = -1;
  } else {
    buff = buff.substr(s+1);
    buff_base = buff_base + s + 1;
  }
  
  return *this;
}

}} // namespace
