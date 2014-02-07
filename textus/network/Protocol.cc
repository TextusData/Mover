/* Protocol.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
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
