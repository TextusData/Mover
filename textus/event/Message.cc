/* Message.cc -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * This class represents a message.  Usually a wrapper around a protobuf.
 */

#include "textus/event/Message.h"
#include "textus/base/init/Init.h"

#include <string>

namespace textus { namespace event {

using namespace std;

MessageBuilder::mb_map Message::message_builders;

Message *Message::build(uint32_t type, string data) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&message_builders);
  uint32_t mask = 0xff;
  uint32_t orig_type = type;
  MessageBuilder::mb_map::iterator i;
  for (i = message_builders.find(type); i == message_builders.end() && type != 0; i = message_builders.find(type)) {
    type &= ~mask;
    mask <<= 8;
  }

  if (type == 0 || i == message_builders.end()) {
    return NULL;
  }

  Message *m = i->second->build(orig_type, data);

  if (m && m->type() == 0) {
    m->setType(orig_type);
  }

  return m;
}

void Message::registerBuilder(uint32_t t, MessageBuilder *mb) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&message_builders);
  MessageBuilder::mb_map::iterator i;
  i = message_builders.find(t);
  if (i != message_builders.end()) {
    LOG(ERROR) << "Registering a second message builder for the same type " << t << ".\n";
    die();
  }
  message_builders[t] = mb;
}

void Message::eraseBuilder(uint32_t t) {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&message_builders);
  message_builders.erase(t);
}


std::string Message::setChecksum(std::string d) {
  size_t offset = (d[0] & 7) + 1;
  assert (d.length() > MESSAGE_CHECKSUM_OFFSET + 3 + offset);
  uint32_t csum = checksum(d);
  d[MESSAGE_CHECKSUM_OFFSET + offset] = static_cast<char>((csum >> 24) & 0xff);
  d[MESSAGE_CHECKSUM_OFFSET + offset + 1] = static_cast<char>((csum >> 16) & 0xff);
  d[MESSAGE_CHECKSUM_OFFSET + offset + 2] = static_cast<char>((csum >> 8) & 0xff);
  d[MESSAGE_CHECKSUM_OFFSET + offset + 3] = static_cast<char>((csum >> 0) & 0xff);
  return d;
}

uint32_t Message::checksum(std::string data) {
  size_t offset = (data[0] & 7) + 1;
  assert (data.length() > MESSAGE_CHECKSUM_OFFSET + 3 + offset);

  uint32_t accumulator = 0;
  uint32_t val = 0;

  data[MESSAGE_CHECKSUM_OFFSET + 0 + offset] = 0;
  data[MESSAGE_CHECKSUM_OFFSET + 1 + offset] = 0;
  data[MESSAGE_CHECKSUM_OFFSET + 2 + offset] = 0;
  data[MESSAGE_CHECKSUM_OFFSET + 3 + offset] = 0;

  for (unsigned int i = 0; i < data.length(); ++i) {
    val = val << 8;
    val = val + data[i];
    if ((i & 3) == 0) {
      accumulator = accumulator + val;
      val = 0;
    }
  }

  accumulator = accumulator + val;
  // mask off the high byte, so we can
  // reserve it for checksum type later.
  accumulator += accumulator >> 24;
  accumulator &= 0x00FFFFFF;
  return accumulator;
}

}} // namespace
