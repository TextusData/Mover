/* FileHandleEventFactory.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Builds events for file handles.
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

#ifndef TEXTUS_FILE_FILE_HANDLE_EVENT_FACTORY_H_
#define TEXTUS_FILE_FILE_HANDLE_EVENT_FACTORY_H_

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/base/Error.h"
#include "textus/event/EventFactory.h"
#include "textus/file/HandleEventFactory.h"

namespace textus { namespace file {
using namespace textus::base;
using namespace textus::event;


class FileHandleEventFactory: public HandleEventFactory {
public:
  FileHandleEventFactory() {}
  virtual ~FileHandleEventFactory() {}
  virtual Event *errorEvent(Error *e) { return NULL;  }
  virtual Event *writeEvent() { return NULL; }
  virtual Event *readEvent(string s) { return NULL; } 
  virtual Event *eofEvent() { return NULL; }
};

}} //namespace

#endif //TEXTUS_FILE_FILE_HANDLE_EVENT_FACTORY_H_
