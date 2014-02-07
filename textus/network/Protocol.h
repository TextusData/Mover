/* Protocol.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
 *
 */

#ifndef TEXTUS_NETWORK_PROTOCOL_H_
#define TEXTUS_NETWORK_PROTOCOL_H_
#include "textus/base/Base.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/WeakReferenceList.h"

#include <string>

namespace textus { namespace network {

class ProtocolServer;
class ProtocolHandle;
class ProtocolSocket;

using namespace textus::base;
using namespace std;

class Protocol: virtual public Base {
private:
  static ReferenceValueMap<string, Protocol *> registered_protocols;
  WeakReferenceList<ProtocolSocket *> connections;
protected:

public:
  static Protocol *registerProtocol(string name, Protocol *proto);
  static Protocol *defaultProtocol() { return NULL; }
  static void unregisterProtocol(string name);

  static Protocol *findProtocol(string name);

  static Protocol *getFileProtocol();

  Protocol();
  virtual ~Protocol();
  virtual ProtocolSocket *connect(string hostname, int port)=0;
  virtual ProtocolServer *listen()=0;

  virtual ProtocolHandle *sendRequest(ProtocolSocket *, string path)=0;
  
  
};


}} //namespace


#endif // TEXTUS_NETWORK_PROTOCOL_H_
