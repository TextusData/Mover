/* FileEvent.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Base class for events related to files.
 * Handles a bunch of common cases.
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
