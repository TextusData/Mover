/* Actor.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * An Actor represents an object that acts on messages received from
 * file handles or other places.
 *
 */

#ifndef TEXTUS_EVENT_ACTOR_H_
#define TEXTUS_EVENT_ACTOR_H_

#include "textus/base/Base.h"

namespace textus { namespace event {

class Message;

class Actor: public virtual textus::base::Base {
private:
public:
  Actor() {}
  virtual ~Actor() {}
  
  virtual void act(Message *)=0;
  virtual void close() {}
};

}} //namespace

#endif //TEXTUS_EVENT_ACTOR_H_
