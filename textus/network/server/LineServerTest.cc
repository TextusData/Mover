/* LineServerTest.cc -*- c++ -*-
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
#include "textus/network/server/SimpleServer.h"
#include "textus/network/server/LineServerSocket.h"
#include "textus/base/AutoDeref.h"

using namespace textus::testing;
using namespace textus::network::server;

namespace textus { namespace network { namespace server {

class TestProcessor : virtual public Processor {
private:
  AutoWeakDeref< LineServerSocket<TestProcessor> > parent;

public:
  explicit TestProcessor(LineServerSocket<TestProcessor> *p) {
    parent = p;
  }
  virtual ~TestProcessor() {}

  virtual void processMessage(std::string s) {
    Synchronized(this);
    if (s.substr(0, 5) == "hello") {
      parent->sendData("GoodBye\n");
      parent->close();
    } else {
      parent->sendData("What?\n");
    }
  }

  virtual void processMessage(textus::event::Message *m) {
    Processor::processMessage(m);
  }

};


class LineServerTest: public UnitTest {
private:

public:
  LineServerTest();
  ~LineServerTest();

  void run_tests();
};

LineServerTest::LineServerTest(): UnitTest(__FILE__)
{
}

LineServerTest::~LineServerTest()
{
}

void LineServerTest::run_tests() {
  fprintf (stderr, "Testing line server socket...");

  AUTODEREF(SimpleServer *, ss);
  ss = new SimpleServer();
  AUTODEREF(LineServerFactory<TestProcessor> *, lsf);
  lsf = new LineServerFactory<TestProcessor>();
  ss->setSocketFactory(lsf);

  AUTODEREF(NetworkAddress *, server);
  server = new NetworkAddress(AF_INET, htonl(INADDR_LOOPBACK), 0 );
  ss->setAddress(server);
  server->deref();

  socklen_t slen;
  const sockaddr_in *sin = reinterpret_cast<const sockaddr_in *>(ss->address()->sockAddress(&slen));
						     
  server = new NetworkAddress(AF_INET, htonl(INADDR_LOOPBACK), sin->sin_port);

  AUTODEREF(Socket *, s);
  s = new Socket();

  AUTODEREF(SocketEventFactory *, sef);
  sef = new SocketEventFactory(s);
  s->setEventFactory(sef);

  s->connect(server);
  if (s->waitForConnect()) {
    fprintf (stderr, "connect failed.\n");
    assert(0);
  }

  string str = "hello\n";
  s->sendData(str);
  string str2 = s->waitForData();
  assert(strcmp("GoodBye\n", str2.c_str()) == 0);
  fprintf (stderr, "Ok\n");
  s->close();
  ss->close();
}

}}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new LineServerTest();
}
