/* ProtoBufReaderEventFactory.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * A class to process events for reading a file one line at a time.
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

#include "textus/file/ProtoBufReaderEventFactory.h"
#include "textus/file/ProtoBufReaderEvent.h"

namespace textus { namespace file {

Event *ProtoBufReaderEventFactory::protoBufEvent() {
  Synchronized(this);
  ProtoBufReaderEvent *lre = new ProtoBufReaderEvent(parent);
  setEventQueue(lre);
  return lre;
};

}} //namespace
