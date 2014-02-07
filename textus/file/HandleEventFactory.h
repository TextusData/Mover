/* FileHandleEventFactory.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Builds events for file handles.
 *
 */

#ifndef TEXTUS_FILE_HANDLE_EVENT_FACTORY_H_
#define TEXTUS_FILE_HANDLE_EVENT_FACTORY_H_

#include "textus/base/Base.h"
#include "textus/event/Event.h"
#include "textus/base/Error.h"
#include "textus/event/EventFactory.h"

namespace textus { namespace file {
using namespace textus::base;
using namespace textus::event;


class HandleEventFactory: public EventFactory {
public:
  HandleEventFactory() {}
  virtual ~HandleEventFactory() {}
  virtual Event *errorEvent(Error *e) { return NULL;  }
  virtual Event *writeEvent() { return NULL; }
  virtual Event *readEvent(string s) { return NULL; } 
  virtual Event *eofEvent() { return NULL; }
};

}} //namespace

#endif //TEXTUS_FILE_HANDLE_EVENT_FACTORY_H_
