/* FileProtocol.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with files as a network protocol.
 *
 */

#include "textus/network/protocols/FileProtocol.h"

namespace textus { namespace network { namespace protocols {

ProtocolSocket *FileProtocol::connect(string hostname, int port) {
  if (hostname.length() != 0 || port != 0) {
    return NULL;
  }

  ProtocolSocket *ps = new ProtocolSocket();
  ps->setProtocol(this);
  return ps;
}



}}} //namespace
