/* Mover.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * Main class for an app that copies chunks of data around.
 * In general, those chunks will be encrypted messages.
 * This should be thought of as similar to NNTP.
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

#ifndef TEXTUS_MOVER_H_
#define TEXTUS_MOVER_H_

#define MOVER_MESSAGE_CATEGORY 0x02
#define MOVER_MESSAGE_TYPE 0x01

#include <set>

#include "textus/base/Base.h"
#include "textus/network/server/SimpleServer.h"
#include "textus/network/server/ServerTemplate.h"
#include "textus/network/ssl/SecureSocket.h"
#include "textus/network/ssl/SecureMessageServer.h"
#include "textus/hash/Hasher.h"
#include "textus/file/HashFileStore.h"
#include "textus/base/UnionSet.h"
#include "textus/event/TimerEvent.h"
#include "textus/mover/MoverVerifier.h"
#include "textus/base/Cache.h"

namespace textus { namespace mover {

using namespace textus::network;
using namespace textus::network::server;
using namespace textus::network::ssl;

class MoverMessageBuilder;
class MoverMessageProcessor;
class Mover;

class MoverTimerEvent: public textus::event::TimerEvent {
private: 
  WMVAR(public, Mover, mover);

protected:
  virtual void timeOut();
  virtual void do_it();

public:
  MoverTimerEvent(Duration d, Mover *m);
  ~MoverTimerEvent();
  void close();
};

DECLARE_INT_ARG(mover_max_connection_time);
DECLARE_INT_ARG(mover_min_connection_time);
DECLARE_INT_ARG(mover_connection_timeout);
DECLARE_INT_ARG(mover_random_message_time);

class Mover: virtual public ServerCoordinator {
private:
  MVAR(protected, SimpleServer, ss);
  MVAR(public, MoverVerifier, verifier);
  MVAR(protected, MoverMessageBuilder, mmb);
  MVAR(protected, Certificate, cert);
  MVAR(protected, SecureMessageServerFactory<MoverMessageProcessor>, mmf);
  MVAR(protected, URL, url);
  MVAR(protected, textus::hash::Hasher, hasher);
  MVAR(protected, textus::file::HashFileStore, file_store);
  MVAR(protected, MoverTimerEvent, timer);
  MVAR(public, TextusFile, have_file);
  Cache<string, int> seen_cache;
  set<string> ihave_;
  ReferenceList<MoverMessageProcessor *> clients;
  bool first_poll;
  bool mover_done;
  bool upload_only;

protected:
  bool isAllowed(MoverMessageProcessor *mmp, string id);
  virtual int weakDelete();
  int expire();

public:
  explicit Mover();
  virtual ~Mover();

  void bindCertificate(std::string certfile);
  int bindServer(string url);
  int listen();

  int encryptRandom(string d, string *out);

  set<string> &ihave() { Synchronized(this); return ihave_;}
  const set<string> &ihave() const {SynchronizedConst(this); return ihave_; }
  void setihave(set<string> &s) {Synchronized(this); ihave_ = s; }
  string whatDoIHave(MoverMessageProcessor *);
  string parseIHave(MoverMessageProcessor *, string);
  void parseIWant(MoverMessageProcessor *, string);
  bool iWant(string);
  void handleExtension(MoverMessageProcessor *, string);
  int maxHelloCount();
  void storeData(string);
  int attachFileStore(string root);
  int uploadFiles(list<string>);
  int uploadData(string);
  const set<string> &iHave() const;
  const set<string> iReallyHave() const;
  std::string stringIHave();
  void addIHave(MoverMessageProcessor *, const set<string> &);
  void addRandom(set<string> &, set<string> &, int len);
  void selectRandom(set<string> &, int num);
  SecureMessageServer<MoverMessageProcessor> *connectToPeer(string peer);
  void pollPeers();
  void startTimer();
  void resetIHave();
  void timeOut();
  void waitForShutdown();
  void setMoverDone(bool flag=true) { Synchronized(this); mover_done = flag; }
  void uploadOnly(bool flag=true) { Synchronized(this); upload_only = flag; }
  int createRandomMessage();
  string addHeader(uint64_t magic, string data, int type);
  string getDataForHash(string hash);
};

#define MOVER_HEADER_TYPE_MESSAGE	1
#define MOVER_HEADER_TYPE_RECURSIVE	2

DECLARE_LIST_ARG(string, mover_upload_files);
DECLARE_STRING_ARG(mover_root);
DECLARE_STRING_ARG(mover_certificate);
DECLARE_BOOL_ARG(mover_loopback_test);
DECLARE_BOOL_ARG(mover_daemon);
DECLARE_STRING_ARG(mover_bind_address);
DECLARE_INT_ARG(mover_max_data_size);
DECLARE_STRING_ARG(mover_have_file_name);
DECLARE_STRING_ARG(mover_user_root);
}} // Namespace

#endif // TEXTUS_MOVER_H_
