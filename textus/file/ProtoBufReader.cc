/* ProtoBufReader.cc -*- c++ -*-
 * Copyright (c) 2010-2014 Ross Biro
 *
 * A class to read a file one Message at a time.
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

#include "textus/file/ProtoBufReader.h"
#include "textus/file/FileEvent.h"
#include "textus/event/Message.h"

#define MAX_MESSAGE_LENGTH ((1 << 24) - 1)

namespace textus { namespace file {

class InternalProtoBufReaderEvent: public textus::event::Event {
private:
  bool eof_flag;
  bool write_flag;
  AutoDeref<Error> err;
  string buffer;

protected:
  void do_it();

public:
  InternalProtoBufReaderEvent(ProtoBufReader *p): Event(p), eof_flag(false), write_flag(false) {}
  virtual ~InternalProtoBufReaderEvent() {}

  bool eof() {
    Synchronized(this);
    return eof_flag;
  }

  void setWrite(bool w) {
    Synchronized(this);
    write_flag = w;
  }

  bool writeFlag() {
    Synchronized(this);
    return write_flag;
  }

  void setEof(bool f) {
    Synchronized(this);
    eof_flag = f;
    LOG(INFO) << "InternalProtoBufReaderEvent setting eof to " << f << "\n";
  }

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

void InternalProtoBufReaderEvent::do_it() {
  Synchronized(this);
  ProtoBufReader *parent = dynamic_cast<ProtoBufReader *>(eventTarget());
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
    if (writeFlag()) {
      parent->write(this);
    }
  }
}

class InternalProtoBufReaderEventFactory: public FileHandleEventFactory {
private:
  AutoWeakDeref<ProtoBufReader> parent;

public:
  explicit InternalProtoBufReaderEventFactory(ProtoBufReader *lr):parent(lr) {}
  virtual ~InternalProtoBufReaderEventFactory() {}
  virtual Event *errorEvent(Error *err) {
    Synchronized(this);
    InternalProtoBufReaderEvent *e;
    e = new InternalProtoBufReaderEvent(parent);
    e->setError(err);
    LOG(INFO) << "InternalProtoBufReaderEventFactory error\n";
    setEventQueue(e);
    return e;
  }

  virtual Event *writeEvent() {
    Synchronized(this);
    InternalProtoBufReaderEvent *e;
    e = new InternalProtoBufReaderEvent(parent);
    LOG(INFO) << "InternalProtoBufReaderEventFactory write\n";
    e->setWrite(true);
    setEventQueue(e);
    return e;
  }

  virtual Event *readEvent(string s) { 
    Synchronized(this);
    InternalProtoBufReaderEvent *e;
    e = new InternalProtoBufReaderEvent(parent);
    e->setData(s);
    LOG(INFO) << "InternalProtoBufReaderEventFactory read\n";
    setEventQueue(e);
    return e;
  } 

  virtual Event *eofEvent() {
    Synchronized(this);
    InternalProtoBufReaderEvent *e;
    e = new InternalProtoBufReaderEvent(parent);
    e->setEof(true);
    LOG(INFO) << "InternalProtoBufReaderEventFactory eof\n";
    setEventQueue(e);
    return e;
  }

};

ProtoBufReader::ProtoBufReader(FileHandle *f): fh(f), eof_flag(false), eof_thread(NULL), read_thread(NULL)
{
  AUTODEREF(InternalProtoBufReaderEventFactory *, lref);
  lref = new InternalProtoBufReaderEventFactory(this);
  fh->setEventFactory(lref);
}

Message *ProtoBufReader::readMessage(ProtoBufReaderStatus::status &status)
{
  Synchronized(this);
  status = ProtoBufReaderStatus::ok;
  // we don't allow NULL messages
  if (buffer.length() > 3*sizeof(uint32_t) + 1) {

    // First byte of a message is either 0, or 
    // a random pad of up to 8 bytes that tells
    // us where the length actually is.
    uint32_t offset = (buffer[0] & 0x7) + 1;

    uint32_t len = 0;
    for (unsigned i = 0; i < sizeof(len); ++i) {
      len = len << 8;
      uint32_t ul = (unsigned char)buffer[offset + i];
      len = len + ul;
    }

    if (len+offset >= MAX_MESSAGE_LENGTH || len+offset <= 3*sizeof(uint32_t)) {
      error();
      fh->close();
      status = ProtoBufReaderStatus::eof;
      return NULL;
    }

    if (buffer.length() >= len+offset) {
      uint32_t type;
      type = (static_cast<unsigned>(buffer[4+offset]) << 24) + 
	(static_cast<unsigned>(buffer[5+offset]) << 16) + 
	(static_cast<unsigned>(buffer[6+offset]) << 8) + 
	static_cast<unsigned>(buffer[7+offset]);

      uint32_t csum;
      csum = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[MESSAGE_CHECKSUM_OFFSET + offset])) << 24) + 
	(static_cast<uint32_t>(static_cast<unsigned char>(buffer[MESSAGE_CHECKSUM_OFFSET + 1 + offset])) << 16) + 
	(static_cast<uint32_t>(static_cast<unsigned char>(buffer[MESSAGE_CHECKSUM_OFFSET + 2 + offset])) << 8) + 
	static_cast<uint32_t>(static_cast<unsigned char>(buffer[MESSAGE_CHECKSUM_OFFSET + 3 + offset]));

      if (csum != Message::checksum(buffer.substr(0, len+offset))) {
	LOG(WARNING) << "Message with bad checksum csum="<< csum << ".  Message has checksum "<< Message::checksum(buffer.substr(0, len+offset)) 
		     << ".\n";
	error();
	fh->close();
	status = ProtoBufReaderStatus::eof;
	return NULL;
      }

      // 0 in any byte is reserved.  Make sure one
      // doesn't try to sneak through.
      for (unsigned int i = 0; i < sizeof(type); i++) {
	if ((type & (0xff << (8*i))) == 0) {
	  LOG(WARNING) << "Message with reserved type: " << type << ".\n";
	  error();
	  fh->close();
	  status = ProtoBufReaderStatus::eof;
	  return NULL;
	}
      }

      Message *m = Message::build(type, buffer.substr(0+offset, len+offset));

      if (m == NULL) {
	error();
	fh->close();
	status = ProtoBufReaderStatus::eof;
	return NULL;
      }

      buffer = buffer.substr(len+offset);
      if (fh) {
	Actor *a = fh->context();
	if (a) {
	  a->act(m);
	}
      }

      if (factory) {
	AUTODEREF(ProtoBufReaderEvent *, e);
	e = dynamic_cast<ProtoBufReaderEvent *>(factory->protoBufEvent());
	e->setMessage(m);
	e->post();
      }

      return m;
    }
  }

  if (eof_flag) {
    status = ProtoBufReaderStatus::eof;
    return NULL;
  }
  status = ProtoBufReaderStatus::wait;
  return NULL;
}

Message *ProtoBufReader::waitForMessage()
{
  Synchronized(this);
  ProtoBufReaderStatus::status stat;
  while (true) {
    Message *m = readMessage(stat);
    if (stat != ProtoBufReaderStatus::wait) {
      if (stat == ProtoBufReaderStatus::eof) {
	// assert(buffer.length() == 0); This happens with an invalid message.
	return NULL;
      } else {
	assert (m != NULL);
      }
      return m;
    }
    wait();
  }
}
    
void ProtoBufReader::error(InternalProtoBufReaderEvent *lre) {
  Synchronized(this);
  assert(0);
  setError(lre->error());
  signal();
}

void ProtoBufReader::write(InternalProtoBufReaderEvent *lre) {
  Synchronized(this);
  // we need to do a null write in case we are dealing with a secure socket.
  if (fh != NULL) {
    fh->canWrite(lre);
  }
  signal();
}

void ProtoBufReader::read(InternalProtoBufReaderEvent *lre) {
  Synchronized(this);
  assert(!eof());
  read_thread = Thread::self();
  buffer = buffer + lre->data();
  if (fh && (factory || fh->context())) {
    while (true) {
      AUTODEREF(Message *, m);
      ProtoBufReaderStatus::status stat;
      m = readMessage(stat);
      if (m == NULL) {
	break;
      }
    }
  }
  signal();
}

void ProtoBufReader::setEventFactory(ProtoBufReaderEventFactory *lref) {
  Synchronized(this);
  factory = lref;
  if (fh->eventFactory()) {
    factory->setQueue(fh->eventFactory()->getQueue());
  } 
}
  
}} //namespace
