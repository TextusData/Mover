/* Protocol.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
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

#include "textus/network/ProtocolSocket.h"
#include "textus/network/Protocol.h"

namespace textus { namespace network {

ReferenceValueMap<string, Protocol *> Protocol::registered_protocols;

Protocol *Protocol::registerProtocol(string name, Protocol *proto) {
  registered_protocols[name] = proto;
  return proto;
}

void Protocol::unregisterProtocol(string name) {
  ReferenceValueMap<string, Protocol *>::iterator it;
  it = registered_protocols.find(name);
  if (it != registered_protocols.end()) {
    registered_protocols.erase(it);
  }
}


Protocol *Protocol::findProtocol(string name) {
    if (registered_protocols.count(name)) {
	return registered_protocols[name];
    }
    return NULL;
  }


Protocol *Protocol::getFileProtocol() {
    return findProtocol("file");
  }


}} //namespace
