/* ProtoBufObjectTest.cc -*- c++ -*-
 * Copyright (c) 2010-2013
 *
 * Test the ProtoBufObject class.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/file/FileHandle.h"
#include "textus/persist/ProtoBufObject.h"

#include "textus/persist/TestProtoMessage.pb.h"

using namespace textus::testing;
using namespace textus::persist;

namespace textus { namespace persist {
using namespace std;

#define TEST_MESSAGE_TYPE 0x01
#define TEST_PROTOBUF_MESSAGE_TYPE 0x02

class TestProtoMessage: public Message {
private:
  int value;

public:
  TestProtoMessage(): value(-1) {}
  TestProtoMessage(const TestProtoMessagePB &pb):value(0) {
    if (pb.has_int_value()) {
      value = pb.int_value();
    }
  }
  
  virtual ~TestProtoMessage() {}
  virtual string marshall() {
    string d;
    TestProtoMessagePB pb;
    pb.set_int_value(value);
    string data;
    pb.SerializeToString(&data);

    d.reserve(3 * sizeof(uint32_t) + data.length()+1);
  
    uint32_t t = type();
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
  
  bool operator == (const TestProtoMessage &t2) const {
    return value == t2.value;
  }
  
};

class TestMessage: public Message {
private:
  uint32_t data;

public:
  TestMessage() {}
  virtual ~TestMessage() {}
  virtual string marshall();
  
  bool operator == (const TestMessage &t2) const {
    return data == t2.data;
  }

  void setData(uint32_t d) {
    data = d;
  }
};

string TestMessage::marshall() {
  string d;
  d.reserve(4 * sizeof(uint32_t) + 1);
  
  uint32_t t = type();
  uint32_t length = 4 * sizeof(uint32_t);
  uint32_t csum = 0;
  
  d.append(1, '\0');
  d = Message::append(d, length);
  d = Message::append(d, t);
  d = Message::append(d, csum);
  d = Message::append(d, data);
  d = Message::setChecksum(d);

  return d;
}

class TestMessageBuilder: public MessageBuilder {
public:
  TestMessageBuilder() {}
  virtual ~TestMessageBuilder() {}
  virtual Message *build(uint32_t type, string data) {
    TestMessage *m = new TestMessage();
    m->setData(Message::getUint32(data, 12));
    return m;
  }
};

class TestProtoMessageBuilder: public MessageBuilder {
public:
  TestProtoMessageBuilder() {}
  virtual ~TestProtoMessageBuilder() {}
  virtual Message *build(uint32_t type, string data) {
    TestProtoMessagePB pb;
    pb.ParseFromString(data.substr(MESSAGE_START_OFFSET));
    TestProtoMessage *m = new TestProtoMessage(pb);
    return m;
  }
};

class ProtoBufObjectTest: public UnitTest {
public:
  ProtoBufObjectTest();
  ~ProtoBufObjectTest();

  void run_tests();
  
};

ProtoBufObjectTest::ProtoBufObjectTest(): UnitTest(__FILE__)
{
}

ProtoBufObjectTest::~ProtoBufObjectTest()
{
}

void ProtoBufObjectTest::run_tests() {
  AUTODEREF(FileHandle *, foo);
  fprintf(stderr, "Testing ProtoBufObject Message...");
  AUTODEREF(Directory *, tmp);
  tmp = getTmpDir();
  assert(tmp != NULL);
  tmp->ref();

  {
    AUTODEREF(TestMessageBuilder *, tmb);
    tmb = new TestMessageBuilder();
    Message::registerBuilder((TEST_MESSAGE_CATEGORY << 24) | (TEST_MESSAGE_TYPE << 16), tmb);

    AUTODEREF(TestProtoMessageBuilder *, tpmb);
    tpmb = new TestProtoMessageBuilder();
    Message::registerBuilder((TEST_MESSAGE_CATEGORY << 24) | (TEST_PROTOBUF_MESSAGE_TYPE << 16), tpmb);

    AUTODEREF(ProtoBufObjectStore *, pbos);
    pbos = new ProtoBufObjectStore();
    assert(pbos != NULL);

    pbos->setBase(tmp);

    PersistantObject::registerObjectStore("test", pbos);

    foo = tmp->openFile("protobuf-object-test", O_WRONLY|O_CREAT);
    assert(foo != NULL);
    AUTODEREF(TestMessage *, message);
    message = new TestMessage();
    message->setType(message->make_type(TEST_MESSAGE_CATEGORY, TEST_MESSAGE_TYPE, 1, 1));
    string data = message->marshall();
    foo->write(data);
    foo->close();
    foo->deref();
    foo = NULL;

    AUTODEREF(ProtoBufObject *, pbo);
    PersistantObject *po = PersistantObject::fetch("test", "protobuf-object-test");
    pbo = dynamic_cast<ProtoBufObject *>(po);

    pbo->waitForValid();
    TestMessage *tm = dynamic_cast<TestMessage *>(pbo->message());
    assert (*tm == *message);
    fprintf(stderr, "ok\n");
  }
  {
    fprintf(stderr, "Testing ProtoBufObject ProtMessage...");

    foo = tmp->openFile("protobuf-object-test2", O_WRONLY|O_CREAT);
    assert(foo != NULL);
    AUTODEREF(TestProtoMessage *, pmessage);
    pmessage = new TestProtoMessage();
    pmessage->setType(pmessage->make_type(TEST_MESSAGE_CATEGORY, TEST_PROTOBUF_MESSAGE_TYPE, 1, 1));
    string data2 = pmessage->marshall();
    foo->write(data2);
    foo->close();
    foo->deref();
    foo = NULL;

    AUTODEREF(ProtoBufObject *, pbo);
    pbo = dynamic_cast<ProtoBufObject *>(PersistantObject::fetch("test", "protobuf-object-test2"));

    pbo->waitForValid();

    TestProtoMessage *tpm = dynamic_cast<TestProtoMessage *>(pbo->message());
    assert (tpm != NULL);
    assert (*tpm == *pmessage);
  }
  fprintf(stderr, "ok\n");
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ProtoBufObjectTest();
}
