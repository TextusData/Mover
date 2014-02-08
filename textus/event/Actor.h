/* Actor.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * An Actor represents an object that acts on messages received from
 * file handles or other places.
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
