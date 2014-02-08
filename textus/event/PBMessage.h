/* PBMessage.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * This template represents a protobuf wrapped in a message.
 *
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

#ifndef TEXTUS_EVENT_PBMESSAGE_H_
#define TEXTUS_EVENT_PBMESSAGE_H_

#include "textus/event/Message.h"
#include "textus/base/init/Init.h"

#include <string>

#define PB_MESSAGE_INIT_PRIORITY 10000

namespace textus { namespace event {

using namespace std;

template <class T, unsigned char pbcategory=0, unsigned char pbtype = 0, unsigned char pbsubtype=0, unsigned char pbmessage=0> 
class PBMessageBuilder: public MessageBuilder {
private:
public:
  PBMessageBuilder() {}
  virtual ~PBMessageBuilder() {}
  virtual Message *build(uint32_t type, string data); 
};

template <class T, unsigned char pbcategory=0, unsigned char pbtype = 0, unsigned char pbsubtype=0, unsigned char pbmessage=0> 
class PBMessage: public Message {
private:
  T pb;
public:
  explicit PBMessage() {} // default construct to build an empty message.
  explicit PBMessage(string data) {
    pb.ParseFromString(data);
  }
  explicit PBMessage(const T& buf) {
     pb = buf;
  }
  virtual ~PBMessage() {}

  void parsePB(string data) {
    pb.ParseFromString(data);
  }

  T &PB() {
    return pb;
  }

  virtual string marshall() {
    string d;
    string data;
    pb.SerializeToString(&data);
    d.reserve(3 * sizeof(uint32_t) + data.length());
  
    uint32_t t = (pbcategory << 24) | (pbtype << 16) | (pbsubtype << 8) | pbmessage;
    assert ( pbcategory != 0 );
    assert ( pbtype != 0 );
    assert ( pbsubtype != 0 );
    assert ( pbmessage !=  0 );
    uint32_t length = 3 * sizeof(uint32_t) + data.length();
    uint32_t csum = 0;
  
    d = Message::append(d, length);
    d = Message::append(d, t);
    d = Message::append(d, csum);
    d = d + data;
    d = Message::setChecksum(d);

    return d;
  }
};


template<class T, unsigned char pbcategory, unsigned char pbtype, unsigned char pbsubtype, unsigned char pbmessage> 
inline Message *PBMessageBuilder<T, pbcategory, pbtype, pbsubtype, pbmessage>::build(uint32_t type, string data) {
  PBMessage<T, pbcategory, pbtype, pbsubtype, pbmessage> *m = 
    new PBMessage<T, pbcategory, pbtype, pbsubtype, pbmessage>(data.substr(MESSAGE_START_OFFSET));
  return m;
}

}} //namespace

#define DECLARE_PB_MESSAGE(name, pbtype, id) extern int force_link_##pbtype ; \
  static __attribute__((unused)) int UNIQUE_IDENTIFIER(fl_##pbtype) = force_link_##pbtype; \
  typedef PBMessage<pbtype, ((id) >> 24), (((id) >> 16) & 0xff), (((id) >> 8) & 0xff), ((id) & 0xff)>  name; \
  typedef PBMessageBuilder<pbtype, ((id) >> 24), (((id) >> 16) & 0xff), (((id) >> 8) & 0xff), ((id) & 0xff)>  name##builder;

#define DEFINE_PB_MESSAGE(name, pbtype, id) int force_link_##pbtype;	\
  DEFINE_INIT_FUNCTION(init_message_##pbtype, PB_MESSAGE_INIT_PRIORITY) { \
    AUTODEREF( name##builder *, tm); \
    tm = new PBMessageBuilder<pbtype, ((id) >> 24), (((id) >> 16) & 0xff), (((id) >> 8) & 0xff), ((id) & 0xff)>(); \
    Message::registerBuilder(id, tm);					\
    return 0;								\
  }

#define OPTIONAL_FIELD(message, name) do { if ((message)->PB().has_##name ()) { set##name((message)->PB().name()); } } while (0)
#define REQUIRED_FIELD(message, name) do { if ((message)->PB().has_##name ()) { set##name((message)->PB().name()); } else { goto error_out;} } while (0)
#define OPTIONAL_CLASS(message, type, name) do { if ((message)->PB().has_##name ()) { set##name(new type((message)->PB().name())); } } while (0)
#define REQUIRED_CLASS(message, type, name) do { if ((message)->PB().has_##name ()) { set##name(new type((message)->PB().name())); } else { goto error_out;} } while (0)


#endif // TEXTUS_EVENT_PBMESSAGE_H_
