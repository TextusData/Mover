/* Reflector.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * The main class in the reflector app.
 * It keeps track of all the listening
 * public keys, and directs
 * connections to them.
 *
 * Turns out this is just tor.
 *
 */

#ifndef TEXTUS_REFLECTOR_H_
#define TEXTUS_REFLECTOR_H_

namespace textus { namespace reflector {

using namespace textus::network;
using namepsace texuts::network::server;
using namespace textus::network::ssl;

class Reflector: virtual public Base {
private:
  MVAR(protected, SimpleServer, ss);
  MVAR(protected, ReflectorMessageBuilder, rmb);
protected:
public:
  explicit Reflector() {}
  virtual ~Reflector() {}

  int bindCertificate(std::string certfile);
  int bindServer(int port);
  int listen();
};

}} // namespace

#endif // TEXTUS_REFLECTOR_H_
