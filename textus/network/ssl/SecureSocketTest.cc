/* SecureSocketTest.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Test the Secure Socket class.
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
#include <limits.h>

#include "textus/event/EventQueue.h"
#include "textus/network/ListeningSocket.h"
#include "textus/base/Base.h"
#include "textus/testing/UnitTest.h"
#include "textus/base/Thread.h"
#include "textus/event/Event.h"
#include "textus/network/ssl/SecureSocket.h"
#include "textus/system/Environment.h"

using namespace textus::testing;
using namespace textus::network::ssl;
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
  virtual SecureSocket *createSecureSocket();
};

SecureSocket *EchoServerSocketFactory::createSecureSocket() {
  EchoServerSocket *s = new EchoServerSocket();
  AUTODEREF(SocketEventFactory *, f);
  f = new SocketEventFactory(s);
  s->setEventFactory(f);
  s->initTLS();
  s->setCiphers("HIGH");
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
  char buff[PATH_MAX];
  char *cwd = getcwd(buff, sizeof(buff));
  fprintf (stderr, "cwd = %s\n", cwd);
  fprintf (stderr, "Testing secure echo socket...");

  AUTODEREF(ListeningSocket *, ls);
  ls = new ListeningSocket();

  AUTODEREF(EchoServerSocketFactory *, factory);
  factory = new EchoServerSocketFactory();
  ls->setSocketFactory(factory);

  AUTODEREF(SecureSocket *, s);
  s = new SecureSocket();
  s->initTLS();

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
  s->close();
  ls->close();

}

}}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new SocketTest();
}
