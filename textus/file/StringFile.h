/* StringFile.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Use a string as the backing for a file handle.
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
#ifndef TEXTUS_FILE_STRINGFILE_H_
#define TEXTUS_FILE_STRINGFILE_H_

#include <string>

namespace textus { namespace file {
using namespace std;

class StringFile: public Handle {
private:
  string buffer;
  size_t offset;

protected:
  virtual void sawEOF() {}

  virtual void error() {}

  virtual void spaceAvailable() {}

  virtual void dataAvailable() {}

public:
  explicit StringFile():offset(0) {}
  explicit StringFile(string b): buffer(b), offset(0) {}
  virtual ~StringFile() {}

  void addData(string n) {
    Synchronized(this);
    buffer = buffer + n;
  }


  virtual string read(int maxlen) {
    Synchronized(this);
    if (offset + (unsigned)maxlen > buffer.size()) {
      maxlen = buffer.size() - offset;
      eof_flag = true;
    }
    string res = buffer.substr(offset, maxlen);
    offset = offset + maxlen;
    return res;
  }

  virtual int write(string data) {
    Synchronized(this);
    if (buffer.size() - offset > data.size()) {
      buffer = buffer.substr(0, offset) + data
	+ buffer.substr(offset+data.size());
    } else {
      buffer = buffer.substr(0, offset) + data;
    }
    offset = offset + data.length();
    return data.length();
  }

  virtual void close() {
    buffer = "";
    eof_flag = true;
  }

  virtual off_t seek(off_t o, int whence = SEEK_SET) {
    switch (whence) {
    case SEEK_SET:
      if (o < (off_t)buffer.size()) {
	offset = o;
      } else {
	offset = buffer.size();
      }
      return offset;
      
    case SEEK_CUR:
       offset = offset + o;
       if (offset > buffer.size()) {
	 if (o < 0) {
	   offset = 0;
	 } else {
	   offset = buffer.size();
	 }
       }
       return offset;

     case SEEK_END:
       offset = buffer.size() + o;
       if (offset > buffer.size()) {
	 if (o < 0) {
	   offset = 0;
	 } else {
	   offset = buffer.size();
	 }
       }
       return offset;

    default:
      LOG(ERROR) << "Seek called with uknown whence.\n";
      die();
      return 0;
    }
  }
};

}} //namespace


#endif //TEXTUS_FILE_STRINGFILE_H_
