/* SecureSocketTest.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Test the Secure Socket class.
 */
#include "textus/event/EventQueue.h"
#include "textus/network/ListeningSocket.h"
#include "textus/base/Base.h"
#include "textus/testing/UnitTest.h"
#include "textus/base/Thread.h"
#include "textus/event/Event.h"
#include "textus/network/SecureSocket.h"

using namespace textus::testing;
using namespace textus::network;

namespace textus { namespace network { namespace ssl {

class EchoServerSocket: public SecureSocket {
public:
  EchoServerSocket() {}
  virtual ~EchoServerSocket() { }
  virtual void read(SocketEvent *se) {
    sendData(se->data());
  }
  
};

class EchoServerSocketFactory: public SecureSocketFactory {
public:
  EchoServerSocketFactory() {}
  virtual ~EchoServerSocketFactory() {}
  virtual Socket *createSocket();
};

Socket *EchoServerSocketFactory::createSocket() {
  EchoServerSocket *s = new EchoServerSocket();
  AUTODEREF(SocketEventFactory *, f);
  f = new SocketEventFactory(s);
  s->setEventFactory(f);
  return s;
}

class SocketTest: public UnitTest {
private:

public:
  SocketTest();
  ~SocketTest();

  void run_tests();
};

SocketTest::SocketTest()
{
}

SocketTest::~SocketTest()
{
}

void SocketTest::run_tests() {
  fprintf (stderr, "Testing echo socket...");

  AUTODEREF(ListeningSocket *, ls);
  ls = new ListeningSocket();
  AUTODEREF(EchoServerSocketFactory *, factory);
  factory = new EchoServerSocketFactory();
  ls->setSocketFactory(factory);

  AUTODEREF(Socket *, s);
  s = new SecureSocket();

  AUTODEREF(SocketEventFactory *, sef);
  sef = new SocketEventFactory(s);
  s->setEventFactory(sef);

  AUTODEREF(NetworkAddress *, na);
  na = new NetworkAddress();
  ls->bind(na);
  na->deref();
  na = ls->getAddress();

  AUTODEREF(NetworkAddress *, server);
  
  socklen_t slen;
  const sockaddr *sa = na->sockAddress(&slen);
  const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in *>(sa);

  server = new NetworkAddress(AF_INET, htonl(INADDR_LOOPBACK), sin->sin_port );

  s->connect(server);
  if (s->waitForConnect()) {
    fprintf (stderr, "connect failed.\n");
    assert(0);
  }

  string str = "Hello";
  s->sendData(str);
  string str2 = s->waitForData();
  assert(strcmp(str.c_str(), str2.c_str()) == 0);
  fprintf (stderr, "Ok\n");

}

}}} //namespace

UnitTest *createTestObject(int argc, char *argv[]) {
  return new SecureSocketTest();
}
