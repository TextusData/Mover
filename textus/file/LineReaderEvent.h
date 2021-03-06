/* LineReaderEvent.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to read a file one line at a time.
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

#ifndef TEXTUS_FILE_LINE_READER_EVENT_H_
#define TEXTUS_FILE_LINE_READER_EVENT_H_

#include "textus/base/Base.h"
#include "textus/file/LineReader.h"
#include "textus/event/Event.h"

namespace textus { namespace file {

class LineReader;

class LineReaderEvent: public textus::event::Event {
private:
  bool eof_flag;
  string buffer;
  AutoDeref<Error> err;

protected:
  void do_it();

public:
  explicit LineReaderEvent(LineReader *lr);
  virtual ~LineReaderEvent() {}

  LineReader *lineReader();

  void setLineReader(LineReader *lr);

  bool eof() {
    Synchronized(this);
    return eof_flag;
  }

  void setEof(bool e) {
    Synchronized(this);
    eof_flag = e;
  }

  void eof(LineReaderEvent *lre) {}

  Error *error() {
    Synchronized(this);
    return err;
  }

  void setError(Error *e) {
    Synchronized(this);
    err = e;
  }

  string data() {
    Synchronized(this);
    return buffer;
  }

  void setData(string s) {
    Synchronized(this);
    buffer = s;
  }

};

}} // namespace

#endif // TEXTUS_FILE_LINE_READER_EVENT_H_
