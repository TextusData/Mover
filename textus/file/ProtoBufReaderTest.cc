/* ProtoBufReaderTest.cc -*- c++ -*-
 * Copyright (c) 2010, 2013
 *
 * Test the ProtoBufReader class.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "textus/testing/UnitTest.h"
#include "textus/file/FileHandle.h"
#include "textus/file/ProtoBufReader.h"

#include "textus/file/TestProtoMessage.pb.h"

using namespace textus::testing;
using namespace textus::file;

namespace textus { namespace file {
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
  virtual string marshall(uint8_t category, uint8_t type) {
    string d;
    TestProtoMessagePB pb;
    pb.set_int_value(value);
    string data;
    pb.SerializeToString(&data);

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
  virtual string marshall(uint8_t category, uint8_t type);
  
  bool operator == (const TestMessage &t2) const {
    return data == t2.data;
  }

  void setData(uint32_t d) {
    data = d;
  }
};

string TestMessage::marshall(uint8_t category, uint8_t type) {
  string d;
  d.reserve(4 * sizeof(uint32_t)+1);
  
  uint32_t t = (category << 24) | (type << 16) | (1 << 8) | 1;
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

class ProtoBufReaderTest: public UnitTest {
public:
  ProtoBufReaderTest();
  ~ProtoBufReaderTest();

  void run_tests();
  
};

ProtoBufReaderTest::ProtoBufReaderTest(): UnitTest(__FILE__)
{
}

ProtoBufReaderTest::~ProtoBufReaderTest()
{
}

void ProtoBufReaderTest::run_tests() {
  AUTODEREF(TestMessageBuilder *, tmb);
  tmb = new TestMessageBuilder();
  Message::registerBuilder((TEST_MESSAGE_CATEGORY << 24) | (TEST_MESSAGE_TYPE << 16), tmb);

  AUTODEREF(TestProtoMessageBuilder *, tpmb);
  tpmb = new TestProtoMessageBuilder();
  Message::registerBuilder((TEST_MESSAGE_CATEGORY << 24) | (TEST_PROTOBUF_MESSAGE_TYPE << 16), tpmb);

  for (int i = 0; i < 10; i ++) {
    AUTODEREF(TextusFile *, foo);
    fprintf(stderr, "Testing ProtoBufReader Message...");
    AUTODEREF(Directory *, tmp);
    tmp = getTmpDir();
    assert(tmp != NULL);
    tmp->ref();

    {
      foo = tmp->openFile("protobuf-reader-test", O_WRONLY|O_CREAT);
      assert(foo != NULL);
      AUTODEREF(TestMessage *, message);
      message = new TestMessage();
      string data = message->marshall(TEST_MESSAGE_CATEGORY, TEST_MESSAGE_TYPE);
      foo->write(data);
      foo->close();
      foo->deref();

      foo = tmp->openFile("protobuf-reader-test", O_RDONLY);
      assert(foo != NULL);
      FileHandle *fh = dynamic_cast<FileHandle *>(foo);
      AUTODEREF(ProtoBufReader *, lr);
      lr = new ProtoBufReader(fh);
      foo->deref();
      foo = NULL;

      AUTODEREF(Message *, m);
      m = lr->waitForMessage();
      assert(m != NULL);
      TestMessage *tm = NULL;
      tm = dynamic_cast<TestMessage *>(m);
      assert(tm != NULL);

      assert (*tm == *message);
      fprintf(stderr, "ok\n");
    }
    {
      fprintf(stderr, "Testing ProtoBuf Message...");

      foo = tmp->openFile("protobuf-reader-test2", O_WRONLY|O_CREAT);
      assert(foo != NULL);
      AUTODEREF(TestProtoMessage *, pmessage);
      pmessage = new TestProtoMessage();
      string data2 = pmessage->marshall(TEST_MESSAGE_CATEGORY, TEST_PROTOBUF_MESSAGE_TYPE);
      foo->write(data2);
      foo->close();
      foo->deref();

      foo = tmp->openFile("protobuf-reader-test2", O_RDONLY);
      assert(foo != NULL);
      FileHandle *fh = dynamic_cast<FileHandle *>(foo);
      AUTODEREF(ProtoBufReader *, plr);
      plr = new ProtoBufReader(fh);
      foo->deref();
      foo = NULL;

      AUTODEREF(Message *, pm);
      pm = plr->waitForMessage();
      assert(pm != NULL);

      TestProtoMessage *tpm = dynamic_cast<TestProtoMessage *>(pm);
      assert (tpm != NULL);
      assert (*tpm == *pmessage);
      fprintf(stderr, "ok\n");
    }
  }
}

}} //namespace


UnitTest *createTestObject(int argc, const char *argv[]) {
  return new ProtoBufReaderTest();
}
