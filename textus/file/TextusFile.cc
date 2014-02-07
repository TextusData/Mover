/* SystemFile.cc -*- c++ -*-
 * Copyright (c) 2009=2014 Ross Biro
 *
 * Represents a system file.
 *
 */

#include <sys/stat.h>

#include "textus/file/TextusFile.h"
#include "textus/file/SystemFile.h"

#define MAX_ITERATOR_READ 16384

namespace textus { namespace file {

string TextusFile::pathJoin(const string p1, const string p2) {
  if (p1.length() == 0) {
    return FILE_SEP_STRING + p2;
  }
  if (p1[p1.length() - 1] == FILE_SEP_CHAR) {
    return p1 + p2;
  }
  return p1 + FILE_SEP_CSTR + p2;
}

TextusFile *TextusFile::openWithPath(const string path, const string name,
				     int acces, int md) {
  if (name[0] != FILE_SEP_CHAR) {
    return openFile(pathJoin(path, name), acces, md);
  } else {
    return openFile(name, acces, md);
  }
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
