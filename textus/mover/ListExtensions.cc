/* ListExtensions.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
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

#include "textus/mover/ListExtensions.h"

#define REQUEST_UUID "cfbf2220-1688-11e3-8ffd-0800200c9a66"
#define REPLY_UUID "e1bcffb0-1688-11e3-8ffd-0800200c9a66"

namespace textus { namespace mover {

void ListExtensionsRequest::processIncoming(Mover *mover, MoverMessageProcessor *mmp, std::string data) {
  set<UUID> s = MoverExtension::registeredExtensions();
  
  mmp->sendExtension(string(REPLY_UUID)+string("\n")+join(s, "\n"));
}

void ListExtensionsReply::processIncoming(Mover *mover, MoverMessageProcessor *mmp, std::string data) {
  list<string> l = split(data,'\n');
  set<UUID> ext;
  for (list<string>::const_iterator i = l.begin(); i != l.end(); ++i) {
    ext.insert(UUID(*i));
  }
  mmp->setExtensions(ext);
}

DEFINE_INIT_FUNCTION(listExtensionInit, LATE_INIT_PRIORITY) {
  MoverExtension::registerExtension(UUID(string(REQUEST_UUID)), new ListExtensionsRequest());
  MoverExtension::registerExtension(UUID(string(REPLY_UUID)), new ListExtensionsReply());
  return 0;
}

}} // namespace
