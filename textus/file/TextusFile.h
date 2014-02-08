/* TextusFile.h -*- c++ -*-
 * Copyright (c) 2009-2014 Ross Biro
 *
 * A class to represnet a simple file.
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

#ifndef TEXTUS_FILE_TEXTUS_FILE_H_
#define TEXTUS_FILE_TEXTUS_FILE_H_

#include <iostream>
#include <iomanip>

#include "textus/base/Base.h"
#include "textus/event/EventWatcher.h"
#include "textus/util/String.h"

#define FILE_SEP /
#define FILE_SEP_CHAR (FILE_SEP_CSTR[0])
#define FILE_SEP_CSTR STRING(FILE_SEP)
#define FILE_SEP_STRING std::string(FILE_SEP_CSTR)

namespace textus { namespace file {

using namespace textus::base;
// XXXXX FIXME:  For some reason tf << string << "\n"; only sees the string.
class TextusFile:public textus::event::EventTarget, public std::streambuf, public std::ostream {
public:
  typedef std::string value_type;
  class iterator: public std::iterator<std::input_iterator_tag, string> {
  private:
    AutoDeref<TextusFile> parent;
    off_t off;
    off_t buff_base;
    string buff;


  protected:
    int readMore();

  public:
    explicit iterator(TextusFile *tf):parent(tf), off(0), buff_base(0) {}
    explicit iterator(TextusFile *tf, off_t o, off_t b): parent(tf),
							 off(o),
							 buff_base(b) {}
    iterator(const iterator &old);
    virtual ~iterator() {}
    iterator& operator++();
    iterator operator++(int) {iterator tmp(*this); operator++(); return tmp;}
    bool operator==(const iterator& rhs) { return parent == rhs.parent && off == rhs.off && buff_base == rhs.buff_base; }
    bool operator!=(const iterator& rhs) { return !(*this == rhs); }
    string operator*();
    off_t offset() const { return buff_base; }
  };

private:
protected:
  virtual std::streamsize  xsputn(const char *s, std::streamsize n) {
    return write(string(s, n));
  }

public:
  TextusFile() { init(this); }
  static TextusFile *openFile(const string name, int access,
			      int mode = 0660);
  static TextusFile *openWithPath(const string path, const string name,
				  int access, int mode = 0660);
  static TextusFile *openFileSearchPath(const list<string> paths,
					const string name, int acces,
					int mode = 0600);
  static bool exists(string name);
  static string pathJoin(const string p1, const string p2);

  iterator begin() { return iterator(this, 0, 0); }
  iterator end() { return iterator(this, -1, -1); }

  virtual ~TextusFile() {}
  virtual string readAll(int len);
  virtual string read(int maxlen) = 0;
  virtual int write(string data) = 0;
  virtual void close() = 0;
  virtual bool eof() = 0;
  virtual off_t seek(off_t o, int whence = SEEK_SET) { LOG(ERROR) << "TextusFile seek called." << "\n"; die(); return -1; }
  virtual off_t tell() { return seek(0, SEEK_CUR); }
  virtual int lock()  { LOG(ERROR) << "TextusFile lock called." << "\n"; die(); return -1; }
  virtual int unlock(){ LOG(ERROR) << "TextusFile unlock called." << "\n"; die(); return -1; }
  virtual int sputc(char c) {
    return xsputn(&c, 1);
  }
  virtual int sync() { return -1; }

  virtual std::streamsize sputn(const char *c, std::streamsize n) {
    return xsputn(c, n);
  }
  
};


}} //namespace

#endif // TEXTUS_FILE_TEXTUS_FILE_H_
