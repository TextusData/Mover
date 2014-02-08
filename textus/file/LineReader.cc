/* LineReader.cc -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
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

#include "textus/file/LineReader.h"
#include "textus/file/FileEvent.h"

namespace textus { namespace file {

class InternalLineReaderEvent: public textus::file::FileEvent {
private:

protected:
  void do_it();

public:
  InternalLineReaderEvent(LineReader *p): FileEvent(p) {}
  virtual ~InternalLineReaderEvent() {}
  
};

void InternalLineReaderEvent::do_it() {
  Synchronized(this);
  LineReader *parent = dynamic_cast<LineReader *>(eventTarget());
  if (parent) {
    if (data() != "") {
      parent->read(this);
    }
    if (error() != NULL) {
      parent->error(this);
    }
    if (eof()) {
      parent->setEof(eof());
    }
  }
}

class InternalLineReaderEventFactory: public FileHandleEventFactory {
private:
  AutoWeakDeref<LineReader> parent;

public:
  explicit InternalLineReaderEventFactory(LineReader *lr):parent(lr) {}
  virtual ~InternalLineReaderEventFactory() {}
  virtual Event *errorEvent(Error *err) {
    Synchronized(this);
    InternalLineReaderEvent *e;
    e = new InternalLineReaderEvent(parent);
    e->setError(err);
    setEventQueue(e);
    return e;
  }

  virtual Event *writeEvent() { return NULL;}
  virtual Event *readEvent(string s) { 
    Synchronized(this);
    InternalLineReaderEvent *e;
    e = new InternalLineReaderEvent(parent);
    e->setData(s);
    setEventQueue(e);
    return e;
  } 
  virtual Event *eofEvent() {
    Synchronized(this);
    InternalLineReaderEvent *e;
    e = new InternalLineReaderEvent(parent);
    e->setEof(true);
    setEventQueue(e);
    return e;
  }

};

LineReader::LineReader(FileHandle *f): fh(f), eof_flag(false)
{
  AUTODEREF(InternalLineReaderEventFactory *, lref);
  lref = new InternalLineReaderEventFactory(this);
  fh->setEventFactory(lref);
}

string LineReader::readLine(LineReaderStatus::status &status)
{
  Synchronized(this);
  string::size_type t;
  status = LineReaderStatus::ok;
  t = buffer.find('\n');
  if (eof_flag && t == string::npos) {
    t = buffer.length();
  }
  if (t != string::npos) {
    string line = buffer.substr(0, t);
    if (t < buffer.length()) {
      buffer = buffer.substr(t+1);
    } else {
      buffer = "";
    }
    if (line[t-1] == '\r') {
      line = line.substr(0, t-1);
    }
    return line;
  }
  if (eof()) {
    status = LineReaderStatus::eof;
    string s = buffer;
    buffer = "";
    return s;
  }
  status = LineReaderStatus::wait;
  return "";
}

string LineReader::waitForLine()
{
  LineReaderStatus::status stat;
  Synchronized(this);
  while (true) {
    string s = readLine(stat);
    if (stat != LineReaderStatus::wait) {
      return s;
    }
    wait();
  }
}
    
void LineReader::error(InternalLineReaderEvent *lre) {
  setError(lre->error());
  signal();
}

void LineReader::read(InternalLineReaderEvent *lre) {
  Synchronized(this);
  assert(!eof_flag);
  buffer = buffer + lre->data();
  for (string::size_type pos = lre->data().find('\n'); pos != string::npos; pos = lre->data().find('\n', pos+1)) {
    if (factory) {
      AUTODEREF(Event *, e);
      e = factory->lineEvent();
      e->post();
    }
  }
  signal();
}

void LineReader::setEof() {
  Synchronized(this);
  eof_flag = true;
  signal();
}

void LineReader::setEventFactory(LineReaderEventFactory *lref) {
  Synchronized(this);
  factory = lref;
  if (fh->eventFactory()) {
    factory->setQueue(fh->eventFactory()->getQueue());
  }
}
  
}} //namespace
