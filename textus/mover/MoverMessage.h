/* MoverMessage.h -*- c++ -*-
 *
 * The messages used in the mover protocol.
 *
 * Current Message Packet on the wire looks like (in network byte order)
 * 1-7 random bytes. (Lower 3 bits of first byte determine number. add 1 for offset of length)
 * 4 byte length
 * 0x01 <-- message category (textus) may be different if covert channels are in use.
 * 0x01 <-- Message type (mover) may be different if covert channels are in use.
 * 0x01 <-- Message subtype (mover subtype) may be different if covert channels are in use.
 * 1 byte actual message (see defines below).
 * 4 byte checksum
 * 4 byte length of random pad.
 * Variable length random pad.  Helps deter packet length analysis.
 * Data for message
 *
 *
 */

#ifndef TEXTUS_MOVER_MOVER_MESSAGE_H_
#define TEXTUS_MOVER_MOVER_MESSAGE_H_

#include "textus/mover/Mover.h"
#include "textus/random/Random.h"
#include "textus/base/init/Init.h"
#include "textus/event/Message.h"

#define MOVER_MESSAGE_SUBTYPE 1

#define MOVER_MESSAGE_DATA		1
#define MOVER_MESSAGE_HELLO 		2
#define MOVER_MESSAGE_GOODBYE		3
#define MOVER_MESSAGE_IHAVE		4
#define MOVER_MESSAGE_IWANT		5
#define MOVER_MESSAGE_DONE		6
#define MOVER_MESSAGE_RANDOM		7
#define MOVER_MESSAGE_EXTENSION		8

#define MOVER_MESSAGES_SUPPORTED string("\xFF")

namespace textus { namespace mover {

DECLARE_INT_ARG(mover_random_data_max);
DECLARE_INT_ARG(mover_random_data_min);

using namespace textus::random;

class MoverMessage: virtual public textus::event::Message {
private:
  string data; // the info to move.  Usually an encrypted blob, but could be a list of available blobs.
  unsigned char subtype;
  unsigned char message;

public:
  explicit MoverMessage(string s): data(s), subtype(MOVER_MESSAGE_SUBTYPE), message(MOVER_MESSAGE_DATA) {}
  explicit MoverMessage(string s, unsigned char st, unsigned char m): data(s), subtype(st), message(m) {}
  virtual ~MoverMessage() {}

  string marshall() {
    return marshall(MOVER_MESSAGE_CATEGORY, MOVER_MESSAGE_TYPE);
  }

  unsigned char messageType() const {
    return message;
  }

  string marshall(uint8_t category, uint8_t type) {
    Synchronized(this);
    string d;
    string r = Random::data(mover_random_data_min, mover_random_data_max);
    // XXXXX FIXME  For debugging only.
    r = string("");

    d.reserve(4 * sizeof(uint32_t) + data.length() + r.length() + 8);
      
    uint32_t t = (category << 24) | (type << 16) | (subtype << 8) | message;
    uint32_t length = 4 * sizeof(uint32_t) + data.length() + r.length();
    uint32_t csum = 0;
    d = Random::head_pad();
    d = Message::append(d, length);
    d = Message::append(d, t);
    d = Message::append(d, csum);
    d = Message::append(d, r.length());
    d = d + r;
    d = d + data;
    d = Message::setChecksum(d);

    return d;
  }

  void setData(string s) {
    Synchronized(this);
    data = s;
  }

  string getData() const {
    SynchronizedConst(this);
    return data;
  }
};

}} //namespace
#endif //TEXTUS_MOVER_MOVER_MESSAGE_H_