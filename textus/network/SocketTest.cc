/* SocketTest.cc -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Test the Socket class.
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
#include "textus/event/EventQueue.h"
#include "textus/network/ListeningSocket.h"
#include "textus/base/Base.h"
#include "textus/testing/UnitTest.h"
#include "textus/base/Thread.h"
#include "textus/event/Event.h"
#include "textus/network/Socket.h"

using namespace textus::testing;
using namespace textus::network;

namespace textus { namespace network {

class EchoServerSocket: public Socket {
public:
  EchoServerSocket() {}
  virtual ~EchoServerSocket() { }
  virtual void read(SocketEvent *se) {
    sendData(se->data());
  }
  
};

class EchoServerSocketFactory: public SocketFactory {
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

SocketTest::SocketTest(): UnitTest(__FILE__)
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
  s = new Socket();

  AUTODEREF(SocketEventFactory *, sef);
  sef = new SocketEventFactory(s);
  s->setEventFactory(sef);

  AUTODEREF(NetworkAddress *, na);
  na = new NetworkAddress();
  ls->bind(na);
  na->deref();
  na = NULL;
  na = ls->getAddress();

  AUTODEREF(NetworkAddress *, server);
  
  socklen_t slen;
  const sockaddr *sa = na->sockAddress(&slen);
  const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in *>(sa);

  server = new NetworkAddress(AF_INET, htonl(INADDR_LOOPBACK), sin->sin_port );

  na->deref();
  na = NULL;

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

}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new SocketTest();
}
