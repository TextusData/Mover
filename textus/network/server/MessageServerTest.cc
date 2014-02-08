/* MessageServerTest.cc -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Test the MessageServer class.
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
#include "textus/base/AutoDeref.h"
#include "textus/file/ProtoBufReader.h"
#include "textus/network/server/MessageServerSocket.h"

using namespace textus::testing;
using namespace textus::network::server;
using namespace textus::file;

#define TEST_MESSAGE_TYPE 0x01

namespace textus { namespace network { namespace server {

class TestMessage: virtual public Message {
private:
  string data;

public:
  TestMessage(string s):data(s) {}
  virtual ~TestMessage() {}

  string str() {
    return data;
  }

  string marshall() {
    return marshall(TEST_MESSAGE_CATEGORY, TEST_MESSAGE_TYPE);
  }

  string marshall(uint8_t category, uint8_t type) {
    string d;
    d.reserve(3 * sizeof(uint32_t) + data.length() + 1);
  
    uint32_t t = (category << 24) | (type << 16) | (1 << 8) | 1;
    uint32_t length = 3 * sizeof(uint32_t) + data.length();
    uint32_t csum = 0;
  
    d.append(1, '\0');
    d = Message::append(d, length);
    d = Message::append(d, t);
    d = Message::append(d, csum);
    d = d + data;
    d = Message::setChecksum(d);

    return d;
  }

};

class TestMessageBuilder: public MessageBuilder {
public:
  TestMessageBuilder() {}
  virtual ~TestMessageBuilder() {}
  virtual Message *build(uint32_t type, string data) {
    TestMessage *m = new TestMessage(data.substr(MESSAGE_START_OFFSET));
    return m;
  }
};

class TestMessageProcessor : virtual public Processor {
private:
  AutoWeakDeref< MessageServerSocket<TestMessageProcessor> > parent;

public:
  explicit TestMessageProcessor(MessageServerSocket<TestMessageProcessor> *p) {
    parent = p;
  }
  virtual ~TestMessageProcessor() {}
  virtual void processMessage(Message *m) {
    Synchronized(this);
    TestMessage *tm = dynamic_cast<TestMessage *>(m);
    if (tm->str() == "Hello") {
      tm = new TestMessage("GoodBye");
      parent->sendMessage(tm);
      parent->close();
      delete tm; 
    } else {
      tm = new TestMessage("What?");
      parent->sendMessage(tm);
      delete tm; 
    }
  }
};


class MessageServerTest: public UnitTest {
private:

public:
  MessageServerTest();
  ~MessageServerTest();

  void run_tests();
};

MessageServerTest::MessageServerTest(): UnitTest(__FILE__)
{
}

MessageServerTest::~MessageServerTest()
{
}

void MessageServerTest::run_tests() {
  fprintf (stderr, "Testing message server socket...");

  AUTODEREF(TestMessageBuilder *, tmb);
  tmb = new TestMessageBuilder();
  Message::registerBuilder((TEST_MESSAGE_CATEGORY << 24) | (TEST_MESSAGE_TYPE << 16), tmb);

  AUTODEREF(SimpleServer *, ss);
  ss = new SimpleServer();
  AUTODEREF(MessageServerFactory<TestMessageProcessor> *, msf);
  msf = new MessageServerFactory<TestMessageProcessor>();
  ss->setSocketFactory(msf);

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

  AUTODEREF(ProtoBufReader *, pbr);
  pbr = new ProtoBufReader(s);

  AUTODEREF(TestMessage *, tm);
  tm = new TestMessage("Hello");

  s->sendData(tm->marshall());

  AUTODEREF(Message *, m);
  m = pbr->waitForMessage();
  assert(m != NULL);

  TestMessage *tm2 = dynamic_cast<TestMessage *>(m);
  assert(tm2 != NULL);
  
  assert(tm2->str() == "GoodBye");
  fprintf (stderr, "Ok\n");

}

}}} //namespace

UnitTest *createTestObject(int argc, const char *argv[]) {
  return new MessageServerTest();
}
