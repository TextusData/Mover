/* ProtocolSocket.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
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
