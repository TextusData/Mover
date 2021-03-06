/* MoverMessageProcssor.h -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro
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

#ifndef TEXTUS_MOVER_MOVER_MESSAGE_PROCESSOR_H_
#define TEXTUS_MOVER_MOVER_MESSAGE_PROCESSOR_H_

#include "textus/mover/Mover.h"
#include "textus/mover/MoverMessage.h"
#include "textus/base/UUID.h"

namespace textus { namespace mover {

class MoverMessageProcessor;

class MoverMessageBuilder: public MessageBuilder {
private:

public:
  MoverMessageBuilder() {}
  virtual ~MoverMessageBuilder() {}
  virtual Message *build(uint32_t type, string data) {
    size_t random_length = Message::getUint32(data, MESSAGE_START_OFFSET);
    MoverMessage *m = new MoverMessage(data.substr(MESSAGE_START_OFFSET + random_length + 4),
				       MESSAGE_SUBTYPE(type), MESSAGE_MESSAGE(type));
    return m;
  }
};

class MMPTimerEvent: public textus::event::TimerEvent {
private:
  WMVAR(public, MoverMessageProcessor, mmp);

protected:
  virtual void timeOut();
  virtual void do_it();

  virtual int weakDelete();

public:
  MMPTimerEvent(Duration d, MoverMessageProcessor *mmp);
  ~MMPTimerEvent();
  void close();
};

class MoverMessageProcessor: virtual public Processor {
private:
  AutoWeakDeref<SecureMessageServer<MoverMessageProcessor> > parent;
  AutoDeref<MMPTimerEvent> timer;
  set<string> ihave;
  int hello_count;
  bool waiting_for_messages;
  bool sending_messages;
  bool sent_i_have;
  set<UUID> extensions;
  Time last_message;
  Time first_message;
  int connection_timeout;
  int max_connection_time;
  MBOOL(protected, need_startup);


protected:
  class Mover *mover() {
    Synchronized(this);
    return dynamic_cast<Mover *>(coordinator());
  };

  int weakDelete () {
    close();
    return Processor::weakDelete();
  }

public:
  explicit MoverMessageProcessor(SecureMessageServer<MoverMessageProcessor> *p);
  virtual ~MoverMessageProcessor();
  
  void sendHello() {
    Synchronized(this);
    AUTODEREF(MoverMessage *, mm);
    mm = new MoverMessage(MOVER_MESSAGES_SUPPORTED, MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_HELLO);
    if (parent) {
      parent->sendMessage(mm);
    }
  }

  void setTimeout(int t) {
    Synchronized(this);
    connection_timeout=t;
    maybeResetTimer(Duration::seconds(t));
  }

  // XXXX Fixme: Doesn't automatically reset timers.
  void setMaxConnectionTime(int t) {
    Synchronized(this);
    max_connection_time=t;
  }

  void maybeResetTimer(Duration d) {
    Synchronized(this);
    if (timer->endTime() > Time::now() + d) {
      resetTimer(d);
    }
  }

  void resetTimer(Duration d) {
    Synchronized(this);
    if (timer) {
      timer->close();
      timer = NULL;
    }
    AUTODEREF(MMPTimerEvent *, te);
    te = new MMPTimerEvent(d, this);
    if (te == NULL) {
      LOG(ERROR) << "MMPTimerEvent: Unable to create random timer.";
      sendGoodBye();
    }
    timer = te;
  }

  void fastTimeout() {
    Synchronized(this);
    setTimeout(connection_timeout/2);
  }

  void close() {
    Synchronized(this);
    if (timer != NULL) {
      AUTODEREF(MMPTimerEvent *, t);
      t = timer;
      t->ref();
      timer = NULL;
      t->close();
    }
    if (parent != NULL) {
      AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, p);
      p = parent;
      p->ref();
      parent=NULL;
      p->close();
    }
    Processor::close();
  }

  const set<string> &iHave() const {
    SynchronizedConst(this);
    return ihave;
  }

  set<string> &iHave() {
    Synchronized(this);
    return ihave;
  }

  set<string> *iHavePtr() {
    Synchronized(this);
    return &ihave;
  }

  void setIHave(const set<string> &s) {
    Synchronized(this);
    ihave = s;
  }

  void onConnect() {
    LOG(INFO) << "Mover accepted connection from: " << parent->getPeerAddress() 
	      << "\n";
    // This will usually get called in the FileHandle thread.  We need
    // to make sure we switch threads.
    Synchronized(this);
    set_need_startup(true);
    if (timer) {
      timer->close();
      timer = NULL;
    }
    AUTODEREF(MMPTimerEvent *, te);
    te = new MMPTimerEvent(Duration::seconds(0), this);
    if (te == NULL) {
      LOG(ERROR) << "MMPTimerEvent: Unable to create startup timer.";
      sendGoodBye();
    }
    timer = te;
    
  }

  void afterConnect() {
    string have;
    if (mover() == NULL) {
      return;
    }

    int r = Random::rand(4);

    switch (r) {
    case 0:
      sendRandom();
      /* Fall Through */

    case 1:
      have = mover()->whatDoIHave(this);
      sendIHave(have);
      break;

    case 2:
      sendRandom();
      /* Fall Through */

    default:
      sendHello();
      break;
    }
  }

  void sendGoodBye() {
    {
      Synchronized(this);
      AUTODEREF(MoverMessage *, mm);
      mm = new MoverMessage("Good Bye", MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_GOODBYE);
      if (parent) {
	parent->sendMessage(mm);
      }
    }
    close();
  }

  void sendMessage(string data) {
    AUTODEREF(MoverMessage *, mm);
    mm = new MoverMessage(data, MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_DATA);
    Synchronized(this);
    if (parent) {
      parent->sendMessage(mm);
    }
  }

  void setExtensions(set<UUID> ext) {
    Synchronized(this);
    extensions = ext;
  }

  void sendMessagesDone() {
    AUTODEREF(MoverMessage *, mm);
    mm = new MoverMessage("All Done", MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_DONE);
    Synchronized(this);
    if (parent) {
      parent->sendMessage(mm);
    }
    sending_messages = false;
    if (!waiting_for_messages) {
      // XXXXX Fixme: Do I always want to hang up?
      sendGoodBye();
    }
  }

  void sendIWant(string data) {
    AUTODEREF(MoverMessage *, mm);
    mm = new MoverMessage(data, MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_IWANT);
    Synchronized(this);
    if (parent) {
      parent->sendMessage(mm);
    }
  }

  void sendExtension(string data) {
    AUTODEREF(MoverMessage *, mm);
    mm = new MoverMessage(data, MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_EXTENSION);
    Synchronized(this);
    if (parent) {
      parent->sendMessage(mm);
    }
  }

  void sendIHave(string data) {
    AUTODEREF(MoverMessage *, mm);
    LOG(INFO) << "Sending I Have: " << data << "\n";
    mm = new MoverMessage(data, MOVER_MESSAGE_SUBTYPE, MOVER_MESSAGE_IHAVE);
    Synchronized(this);
    if (parent) {
      parent->sendMessage(mm);
    }
    sent_i_have = true;
  }

  void sendRandom() {
    AUTODEREF(MoverMessage *, mm);
    AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, p);
    mm = new MoverMessage(Random::data(), MOVER_MESSAGE_SUBTYPE,
			  MOVER_MESSAGE_RANDOM);
    {
      Synchronized(this);
      p = parent;
      if (p) {
	p->ref();
      }
    }

    if (p) {
      p->sendMessage(mm);
    }
  }

  void timeOut() {
    Synchronized(this);
    if (need_startup()) {
      // need to prime the pump.
      set_need_startup(false);
      afterConnect();
      goto reset_timer;
    }

    if (last_message.elapsedTime() > connection_timeout ||
	first_message.elapsedTime() > max_connection_time) {
      sendGoodBye();
      return;
    }
    sendRandom();

  reset_timer:
    resetTimer(Duration::seconds(Random::rand(mover_random_message_time)));
  }

  virtual void processMessage(Message *m) {
    {
      MoverMessage *mm = dynamic_cast<MoverMessage *>(m);
      Synchronized(this);
      last_message = Time::now();
      LOG(DEBUG) << "MMP::processMessage: " << *mm << "\n";
      switch (mm->messageType()) {

      case MOVER_MESSAGE_HELLO:
	{
	  string have;
	  Synchronized(this);
	  if (++hello_count > mover()->maxHelloCount()) {
	    sendGoodBye();
	  }
	  have = mover()->whatDoIHave(this);
	  sendIHave(have);
	  return;
	}
	  
      case MOVER_MESSAGE_GOODBYE:
	close();
	if (parent) {
	  parent->close();
	}
	return;

      case MOVER_MESSAGE_DATA:
	mover()->storeData(mm->getData());
	return;

      case MOVER_MESSAGE_IHAVE:
	{
	  string want;
	  want = mover()->parseIHave(this, mm->getData());
	  LOG(INFO) << "Sending I want: " << want << "\n";
	  if (want != "") {
	    sendIWant(want);
	  } else {
	    waiting_for_messages = false;
	    if (sent_i_have && sending_messages == false) {
	      fastTimeout();
	    }
	  }

	  if (!sent_i_have) {
	    string have = mover()->whatDoIHave(this);
	    sendIHave(have);
	  }
	  return;
	}

      case MOVER_MESSAGE_IWANT:
	// Since it might take time to acess some of the
	// data, we return immediately and use callbacks
	// to send what we are going to send.
	mover()->parseIWant(this, mm->getData());
	return;
	
      case MOVER_MESSAGE_DONE:
	{
	  Synchronized(this);
	  LOG(INFO) << "Mover Done." << "\n";
	  waiting_for_messages = false;
	  if (!sending_messages) {
	    sendGoodBye();
	  }
	  return;
	}

      case MOVER_MESSAGE_EXTENSION:
	mover()->handleExtension(this, mm->getData());
	return;
      
      default:
      case MOVER_MESSAGE_RANDOM:
	return;
      }
    }
  }
};

}} // namespace
#endif // TEXTUS_MOVER_MOVER_MESSAGE_PROCESSOR_H_
