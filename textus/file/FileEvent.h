/* FileEvent.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Base class for events related to files.
 * Handles a bunch of common cases.
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

#ifndef TEXTUS_FILE_FILE_EVENT_H_
#define TEXTUS_FILE_FILE_EVENT_H_

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/file/HandleEvent.h"
#include "textus/base/AutoDeref.h"
#include "textus/event/EventWatcher.h"

namespace textus { namespace file {
using namespace textus::base;

class FileEvent: public HandleEvent {

public:
  FileEvent(textus::event::EventTarget *t): HandleEvent(t){}
  virtual ~FileEvent() {}


};

}} //namespace

#endif // TEXTUS_FILE_FILE_EVENT_H_
