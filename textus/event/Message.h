/* Message.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * This class represents a message.  Usually a wrapper around a protobuf.
 */

#ifndef TEXTUS_EVENT_MESSAGE_H_
#define TEXTUS_EVENT_MESSAGE_H_

#include "textus/base/Base.h"
#include "textus/base/ReferenceValueMap.h"

/*
 * Message Fromat:
 *
 * 32 bit message length in net order. Includes the 4 bytes for the
 * length in the message. Minimum valid length is 13.  12 bytes of
 * header + at least 1 of message. Top 8 bits reserved for message
 * version number.  Must be 0 for now.
 *
 * 32 bit message type in net order.
 *
 * 32 bit checksum in net order.
 *
 * Rest of message.
 *
 */
#define MESSAGE_LENGTH_OFFSET 0
#define MESSAGE_TYPE_OFFSET 4
#define MESSAGE_CHECKSUM_OFFSET 8
#define MESSAGE_START_OFFSET 12
#define MESSAGE_CATEGORY_SHIFT 24
#define MESSAGE_CATEGORY_MASK 0xFF000000
#define MESSAGE_TYPE_SHIFT 16
#define MESSAGE_TYPE_MASK 0x00FF0000
#define MESSAGE_SUBTYPE_SHIFT 8
#define MESSAGE_SUBTYPE_MASK 0x0000FF00
#define MESSAGE_MESSAGE_SHIFT 0
#define MESSAGE_MESSAGE_MASK 0x000000FF

#define MESSAGE_MAKE_TYPE(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define MESSAGE_CATEGORY(a) (((a) & MESSAGE_CATEGORY_MASK) >> MESSAGE_CATEGORY_SHIFT)
#define MESSAGE_TYPE(a) (((a) & MESSAGE_TYPE_MASK) >> MESSAGE_TYPE_SHIFT)
#define MESSAGE_SUBTYPE(a) (((a) & MESSAGE_SUBTYPE_MASK) >> MESSAGE_SUBTYPE_SHIFT)
#define MESSAGE_MESSAGE(a) (((a) & MESSAGE_MESSAGE_MASK) >> MESSAGE_MESSAGE_SHIFT)

/*
 * Message types: (0 in each byte is reserved and used for a builder that handles multiple messages.)
 *
 * bits 0-7   specific message.
 * bits 8-15  message subtype
 * bits 16-23 message type
 * bits 24-31 message category.
 *
 * Reserved Categories:
 * 0x80-0xfe Experimental.
 * 0xff Tests
 * 0x00 Reserved
 * 0x01 Textus
 * 
 */

#define TEST_MESSAGE_CATEGORY 0xFF
#define TEXTUS_MESSAGE_CATEGORY 0x01

#include <string>

namespace textus { namespace event {

class Message;

class MessageBuilder: virtual public textus::base::Base {
private:
public:
  typedef textus::base::ReferenceValueMap<uint32_t, MessageBuilder *> mb_map;

  MessageBuilder() {}
  virtual ~MessageBuilder() {}

  virtual Message *build(uint32_t type, std::string data) = 0;
};

class Message:public virtual textus::base::Base {
private:
  static MessageBuilder::mb_map message_builders;
  uint32_t typ;

public:
  static Message *build(uint32_t type, std::string data);
  static void registerBuilder(uint32_t, MessageBuilder *);
  static void eraseBuilder(uint32_t);
  static uint32_t checksum(std::string data);
  static std::string append(std::string d, uint32_t v) {
    unsigned char buff[sizeof(v)];
    for (int i = sizeof(buff) - 1; i >= 0 ; --i) {
      buff[i] = v & 0xff;
      v = v >> 8;
    }
    return d + std::string(reinterpret_cast<char *>(buff), sizeof(buff));
  }

  static std::string setChecksum(std::string d);

  static uint32_t getUint32(std::string d, std::string::size_type offset) {
    uint32_t acc = 0;
    for (std::string::size_type i = 0; i < sizeof(uint32_t); ++i) {
      acc = acc << 8;
      unsigned tmp = static_cast<unsigned char>(d[i+offset]);
      assert((tmp & ~0XFF) == 0);
      acc += tmp;
    }
    return acc;
  }

  Message():typ(0) {}
  virtual ~Message() {}

  uint32_t make_type(uint8_t cat, uint8_t type, uint8_t subtype, uint8_t mess) {
    return (static_cast<uint32_t>(cat) << 24) | (static_cast<uint32_t>(type) << 16) |
      (static_cast<uint32_t>(subtype) << 8) | static_cast<uint32_t>(mess);
  }
  
  uint8_t category() {
    Synchronized(this);
    return (typ & MESSAGE_CATEGORY_MASK) >> MESSAGE_CATEGORY_SHIFT;
  }

  virtual std::string marshall() {
    LOG(ERROR) << "marhsall called for Message base type.\n";
    die();
    return std::string("");
  }

  void setType(uint32_t t) {
    Synchronized(this);
    typ = t;
  }

  uint32_t type() {
    Synchronized(this);
    return typ;
  }

};

}} //namespace

#endif // TEXTUS_EVENT_MESSAGE_H_
