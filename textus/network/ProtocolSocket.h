/* ProtocolSocket.h -*- c++ -*-
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

#ifndef TEXTUS_NETWORK_PROTOCOLSOCKET_H_
#define TEXTUS_NETWORK_PROTOCOLSOCKET_H_

#include "textus/network/Socket.h"
#include "textus/network/Protocol.h"
#include "textus/network/ProtocolHandle.h"

namespace textus { namespace network {
class ProtocolHandle;

class ProtocolSocket: virtual public Base {
private:
  AutoDeref<Protocol> protocol;
  AutoDeref<Socket> socket;
protected:

public:
  ProtocolSocket() {}
  virtual ~ProtocolSocket() {}

  Socket *getSocket() {
    Synchronized(this);
    return socket;
  }

  Protocol *getProtocol() {
    Synchronized(this);
    return protocol;
  }

  void setProtocol(Protocol *p) {
    protocol = p;
  }

  ProtocolHandle *sendRequest(string path) {
    return protocol->sendRequest(this, path);
  }

  
  
};

}} //namespace


#endif //TEXTUS_NETWORK_PROTOCOLSOCKET_H_
