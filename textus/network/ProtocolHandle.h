/* ProtocolHandle.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with networking protocols.
 *
 */

#ifndef TEXTUS_NETWORK_PROTOCOL_HANDLE_H_
#define TEXTUS_NETWORK_PROTOCOL_HANDLE_H_

namespace textus { namespace network {

using namespace textus::file;
using namespace std;

class ProtocolHandle: virtual public FileHandle {
private:
  AutoDeref<ProtocolSocket> ps;
  string buffer;

protected:
public:
  ProtocolHandle() {}
  virtual ~ProtocolHandle() {}

};

}} //namespace


#endif // TEXTUS_NETWORK_PROTOCOL_HANDLE_H_
