/* FileProtocol.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Class to represent and work with files as a network protocol.
 *
 */

#ifndef TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_
#define TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_

namespace textus { namespace network { namespace protcols {

class FileProtocol: virtual public Protocol {
private:

public:
  FileProtocol() {}
  virtual ~FileProtocol() {}
  virtual ProtocolSocket *connect(string hostname);
  virtual ProcotolServer *listen() {
    return NULL;
  }

};

}}} //namespace

#endif // TEXTUS_NETWORK_PROTCOLS_FILE_PROTOCOL_H_
