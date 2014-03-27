/* ProtoBufReaderEvent.cc -*- c++ -*-
 * Copyright (c) 2010, 2014 Ross Biro
 *
 * A class to read a file one message at a time.
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

#include "ProtoBufReaderEvent.h"

#include <string>

namespace textus { namespace file {

using namespace std;

ProtoBufReaderEvent::ProtoBufReaderEvent(ProtoBufReader *lr):Event(lr), eof_flag(false) {}

ProtoBufReader *ProtoBufReaderEvent::messageReader() {
  return dynamic_cast<ProtoBufReader *>(eventTarget());
}

void ProtoBufReaderEvent::setProtoBufReader(ProtoBufReader *lr) {
  setEventTarget(lr);
}

void ProtoBufReaderEvent::do_it() {
  AUTODEREF(ProtoBufReader *, parent);
  {
    Synchronized(this);
    ProtoBufReader *parent = messageReader();
    if (parent) {
      parent->ref();
      setEventTarget(NULL);
    }
  }
  if (parent) {
    if (eof_flag) {
      parent->setEof(true);
    } else {
      parent->messageRead(this);
    }
  }
}

}} //namespace
