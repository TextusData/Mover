/* Mover.cc -*- c++ -*-
 * Copyright (c) 2013, 2014 Ross Biro 
 *
 * Implementation of a class to move data.
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

#include "textus/base/init/Init.h"
#include "textus/base/Utility.h"
#include "textus/mover/Mover.h"
#include "textus/mover/MoverExtension.h"
#include "textus/random/Random.h"
#include "textus/network/ssl/SecureMessageServer.h"
#include "textus/event/Message.h"
#include "textus/hash/SHA3.h"
#include "textus/file/HashFileStore.h"
#include "textus/base/UUID.h"
#include "textus/mover/MoverMessage.h"
#include "textus/mover/MoverMessageProcessor.h"
#include "textus/event/TimerEvent.h"
#include "textus/util/Hex.h"
#include "textus/mover/MoverEncryption.h"

#include <set>
  

namespace textus { namespace mover {

static unsigned min(unsigned x, unsigned y) {
  if (x < y) {
    return x;
  }
  return y;
}

using namespace textus::random;


DEFINE_INT_ARG(mover_default_port, 4669, "mover_default_port", "The default port to connect to when one is not specified.");
DEFINE_STRING_ARG(mover_bind_address, ":4669", "mover_bind_address", "The address to listen on for new connections.");
DEFINE_STRING_ARG(mover_certificate, "", "mover_certificate",
		  "The certificate path for the ssl connections.  "
		  "Default is empty which means use a random self signed certificate.");
DEFINE_STRING_ARG(mover_root, "/data/mover", "mover_root", "the root directory for the mover file store.");
DEFINE_STRING_ARG(mover_user_root, "$(HOME)/.mover", "mover_user_root", "The directory for the per user configuration and data files.");
DEFINE_STRING_ARG(mover_have_file_name, "seen", "mover_have_file", "The file that keeps track of all the hashes we have seen.");
DEFINE_INT_ARG(mover_max_cache, 1000, "mover_max_cache", "The maximum size to let the cache of seen hashes grow to.");
DEFINE_INT_ARG(mover_max_hello_count, 5, "mover_max_hello_count",
	       "The maximum number of times we might accept a hello message from a given connection.");
DEFINE_INT_ARG(mover_random_message_frequency, 5, "mover_random_message_frequency",
	       "1 in this number (default 5) of chance of sending a random message whenever there is an opertunity.");
DEFINE_INT_ARG(mover_min_i_have, 10, "mover_i_have_min", "The minimum number of hashes to include in an iHave message.");
DEFINE_INT_ARG(mover_max_i_have, 1000, "mover_i_have_max", "The maximum number of hashes to include in an iHave message.");

DEFINE_INT_ARG(mover_min_i_want, 10, "mover_i_want_min", "The minimum number of hashes to include in an iWant message.");

DEFINE_INT_ARG(mover_max_i_want, 1000, "mover_i_want_max", "The maximum number of hashes to include in an iWant message.");

DEFINE_INT_ARG(mover_max_random_message_size, 128000, "mover_max_random_message_size", "The maximum size of a randomly generated message that gets inserted into the system.");
DEFINE_INT_ARG(mover_min_random_message_size, 512, "mover_max_random_message_size", "The maximum size of a randomly generated message that gets inserted into the system.");

DEFINE_STRING_ARG(mover_peers_file, "peers", "mover_peers_file", "The name of the file that holds the list of peers.");
DEFINE_INT_ARG(mover_max_peers_connect, 20, "mover_max_peers_connect", "The maximum number of peers to initiate connections to.");
DEFINE_INT_ARG(mover_min_peers_connect, 5, "mover_min_peers_connect", "The minimum number of peers to initiate connections to.");
DEFINE_BOOL_ARG(mover_connect_immediately, false, "mover_connect_immediately", "If true, causes the mover to immediately poll some of the peers.  Use with caution.  It might allow an information leak.");
DEFINE_INT_ARG(mover_max_connection_time, 1200, "mover_max_connection_time", "The maximum time (in seconds) a connection will get kept open.");
DEFINE_INT_ARG(mover_min_connection_time, 600, "mover_min_connection_time", "The minimum time (in seconds) a connection will be kept open.");
DEFINE_INT_ARG(mover_connection_timeout, 600, "mover_connection_timeout", "How long to hold open a quiet connection.");
DEFINE_INT_ARG(mover_poll_time, 3600, "mover_poll_time", "Maximum time to wait between polling known peers.");
DEFINE_INT_ARG(mover_random_mail_probability, 8, "mover_random_mail_probability", "1 in this are the odds of creating a random mail every poll time.");
DEFINE_INT_ARG(mover_random_message_time, 30, "mover_random_message_time", "Maximum time between sending random messages.");
DEFINE_BOOL_ARG(mover_loopback_test, false, "mover_loopback_test", "Cause mover to create a loopback connection for testing purposes.");
DEFINE_INT_ARG(mover_rerequest_chance, 20, "mover-rerequest-chance", "1 in this is the chance of requesting something we already have to spread some disinformation.");
DEFINE_INT_ARG(mover_max_data_size, 128000, "mover-max-data-size", "The maximum size of a data file to move around.");
DEFINE_LIST_ARG(string, mover_upload_files, "mover-upload-files", "A list of files to upload.", "");
DEFINE_BOOL_ARG(mover_daemon, true, "mover-daemon", "Cause mover to continue running.  Eventually it will also force it into the background");
DEFINE_INT_ARG(mover_expire_time, 3600*24*5, "mover-expire-time", "How long to keep a file around before dropping it.");

using namespace std;

void MoverTimerEvent::timeOut() {
  do_it();
  textus::event::TimerEvent::timeOut();
}

void MoverTimerEvent::do_it() {
  Synchronized(this);
  if (mover() != NULL) {
    mover()->timeOut();
  }
  close();
}

MoverTimerEvent::MoverTimerEvent(Duration d, Mover *m):TimerEvent(d), mover_(m) {}
MoverTimerEvent::~MoverTimerEvent() { 
  setmover(NULL);
}

void MoverTimerEvent::close() { 
  Synchronized(this);
  setmover(NULL); 
}

Mover::Mover(): first_poll(true), mover_done(false), upload_only(false) {
  seen_cache.set_max_cached(mover_max_cache);
  string config_path =
    textus::system::Environment::getConfigPath("crypto.cfg");
  
  if (MoverEncryption::fromConfigFile(config_path)) {
    LOG(WARNING) << "Unable to read crypto.cfg file.\n";
  }

}

Mover::~Mover() {}

int Mover::createRandomMessage() {
  int ret = 0;
  string d = Random::data(mover_min_random_message_size,
			  mover_max_random_message_size);
  string out;
  HRC(encryptRandom(d, &out));
  file_store()->newData(out);
 error_out:
  return ret;
}

int Mover::attachFileStore(string root) {
  int ret=0;
  Synchronized(this);
  AUTODEREF(Directory *, d);
  sethasher(new textus::hash::SHA3());
  HRNULL(hasher());
  setfile_store(new HashFileStore());
  HRNULL(file_store());
  file_store()->sethashfunc(hasher());
  d = Directory::lookup(root);
  HRNULL(d);
  HRC(file_store()->openRoot(d));
  while (file_store()->size() < (unsigned)mover_min_i_have) {
    HRC(createRandomMessage());
  }
 error_out:
  return ret;
}

int Mover::encryptRandom(string d, string *out) {
  //XXXXXX FixMe, these need to mimic the distribution of
  // real messages.
  int ret = 0;
  static string methods[] = { string("gpg"), string("openssl") };
  int r = Random::rand(ARRAY_SIZE(methods));
  string method = methods[r];
  AUTODEREF(KeyDescription *, keyname);
  MoverEncryption *me = MoverEncryption::findEncryption(method);
  HRNULL(me);
  keyname = me->tempKey();
  HRC(me->newKey(keyname));
  HRC(me->encrypt(keyname, d, out));

 error_out:
  return ret;
}

int Mover::bindServer(string bind_address) {
  int ret = 0;
  Synchronized(this);
  setmmb(new MoverMessageBuilder());
  HRNULL(mmb());
  Message::registerBuilder(MESSAGE_MAKE_TYPE(MOVER_MESSAGE_CATEGORY, MOVER_MESSAGE_TYPE,
					     MOVER_MESSAGE_SUBTYPE, 0), mmb());
  
  setss(new SimpleServer());
  HRNULL(ss());
  ss()->deref();

  setmmf(new SecureMessageServerFactory<MoverMessageProcessor>());
  HRNULL(mmf());
  mmf()->deref();
  mmf()->setTLS(true);
  mmf()->setCiphers(string(SSL_TXT_HIGH));
  mmf()->setcert(cert());
  mmf()->set_coordinator(this);
  mmf()->setverifier(verifier());
  ss()->setSocketFactory(mmf());

  seturl(URL::parseURL(bind_address));
  HRNULL(url());

  ss()->setAddress(url());

  HRC(ss()->waitForBind());

 error_out:
  return ret;
  
}

const set<string> &Mover::iHave() const {
  SynchronizedConst(this);
  return ihave();
}


string Mover::whatDoIHave(MoverMessageProcessor *mmp)
{
  Synchronized(this);
  if (upload_only) {
    return string("");
  }
  return stringIHave();
}

int Mover::expire() {
  int ret = 0;
  AUTODEREF(HashFileStore *,store);
  {
    Synchronized(this);
    store = file_store();
    if (store == NULL) {
      return -1;
    }
    store->ref();
  }
  ret = store->expire(Time::now() - Duration::seconds(mover_expire_time),
		      false);
  resetIHave();
  return ret;
}

void Mover::addIHave(MoverMessageProcessor *mmp, const set<string> &s) {
  Synchronized(this);
  if (mmp != NULL) {
    Synchronized(mmp);
    mmp->setIHave(s);
  }
  ihave().insert(s.begin(), s.end());
}

bool Mover::iWant(string d) {
  Synchronized(this);
  if (seen_cache.find(d) != seen_cache.end()) {
    return false;
  }
  if (file_store()->seen(d)) {
    seen_cache[d] = 1;
    return false;
  }
  return true;
}

string Mover::parseIHave(MoverMessageProcessor *mmp, string m) {
  if (upload_only) {
    return string("");
  }
  stringstream ss(m);
  string item;
  set<string> items;
  string res;
  set<string> needed;
  if (m.find_first_not_of("ABCDEF0123456789\n") != string::npos) {
    mmp->sendGoodBye();
    return "";
  }

  // Have to do two loops here as well to make
  // sure the otherside can't get extra info
  // by putting the same thing in over and over.
  while (getline(ss, item, '\n')) {
    LOG(INFO) << "Parsing I Have: " << item << "\n";
    if (mmp->iHave().count(item) > 0) {
	// I've already told them I have it, so
	// now is not the time to ask for it.
      continue;
    }
    items.insert(item);
  }

  addIHave(mmp, items);

  // With a short list, always ask for everything. 
  // that way you are less likely to leak information.
  if (items.size() < static_cast<unsigned>(mover_min_i_want)) {
    needed = items;
  } else {
    for (set<string>::iterator i = items.begin(); i != items.end(); i++) {
      if (Random::rand(mover_rerequest_chance) == 0 || iWant(*i)) {
	needed.insert(*i);
      }
    }

    unsigned iwant;
    unsigned maxiw = min(static_cast<unsigned>(mover_max_i_want), items.size());
    if (maxiw > static_cast<unsigned>(mover_min_i_want)) {
      iwant = Random::rand(maxiw - mover_min_i_want) + mover_min_i_want;
    } else {
      iwant = maxiw;
    }

    if (needed.size() < iwant) {
      addRandom(needed, items, iwant);
    }

    while (needed.size() > iwant) {
      selectRandom(needed, iwant);
    }
  }

  return join(needed, "\n");
  
}

void Mover::addRandom(set<string> &to, set<string> &from, int len) {
  int ret = 0;
  while (to.size() < static_cast<unsigned>(len)) {
    string *n;
    int count = len - to.size();
    n = Random::choose<set <string> >(from, count);
    HRNULL(n);
    for (int i = 0; i < count; ++i) {
      to.insert(n[i]);
    }
    delete [] n;
  }
 error_out:
  if (!ret) {
    return;
  } else {
    return;
  }
}

bool Mover::isAllowed(MoverMessageProcessor *mmp, string i) {
  Synchronized(this);
  return iHave().count(i) > 0;
}

// side effect: must queue up to send what it is going to send.
// XXXXX Must only send what was offered
void Mover::parseIWant(MoverMessageProcessor *mmp, string wanted) {
  if (upload_only) {
    return;
  }
  if (wanted.find_first_not_of("ABCDEF0123456789\n") != string::npos) {
    mmp->sendGoodBye();
    return;
  }
  stringstream ss(wanted);
  string item;
  set<string> items;
  // Have to parse first to prevent someone
  // from gaining info by sending the
  // same request over and over.
  while (getline(ss, item, '\n')) {
    if (isAllowed(mmp, item)) {
      items.insert(item);
      LOG(DEBUG) << "Parsing I Want: " << item << "\n";
    }
  }
  for (set<string>::iterator i = items.begin(); i != items.end(); i++) {
    mmp->sendMessage(file_store()->getData(item));
  }
  mmp->sendMessagesDone();
}

int Mover::maxHelloCount() {
  return 1+Random::rand(mover_max_hello_count);
}

void Mover::storeData(string data) {
  int ret = 0;
  Synchronized(this);
  if (data.length() > (unsigned)mover_max_data_size) {
    return;
  }
  if (upload_only) {
    return;
  }
  string h = hasher()->computeHash(data);
  h = textus::util::hex_encode(h);
  if (!iWant(h)) {
    return;
  }
  if (have_file() == NULL) {
    AUTODEREF( TextusFile *, tf);
    tf = TextusFile::openDataFile(mover_have_file_name, O_WRONLY|O_CREAT);
    HRNULL(tf);
    HRTRUE(tf->seek(0, SEEK_END) >= 0);
    set_have_file(tf);
  }
  HRTRUE(have_file()->write(h + "\n") == (int)h.length() + 1);
  HRC(have_file()->sync());
  HRC(file_store()->newData(data));
  seen_cache[data] = 1;
 error_out:
  return;
}

void Mover::bindCertificate(string cert) {
  AUTODEREF(Certificate *,c);
  Synchronized(this);
  setcert(NULL); // always throw out any old certificate.
  if (cert.length() == 0) {
    return;
  }
  c = Certificate::fromString(cert);
  if (c == NULL) {
    c = Certificate::fromFile(cert);
  }
  setcert(c);
}

void Mover::resetIHave() {
  Synchronized(this);
  set<string> s = iReallyHave();
  int r = Random::rand(mover_max_i_have - mover_min_i_have) + mover_min_i_have;
  selectRandom(s, r);
  for (ReferenceList<MoverMessageProcessor *>::iterator i = clients.begin(); i != clients.end(); ++i) {
    Synchronized(*i);
    s.insert((*i)->iHave().begin(), (*i)->iHave().end());
  }
  setihave(s);
}

void Mover::selectRandom(set<string> &s, int num) {
  set<string> n;
  string *li = Random::choose< set<string> >(s, num);
  int ret=0;
  HRNULL(li);
  for (int i = 0; i < num; ++i) {
    n.insert(li[i]);
  }
  delete [] (li);
  s = n;
 error_out:
  if (!ret) {
    return;
  } else {
    return;
  }
}

string Mover::stringIHave() {
  return join(iHave(), "\n");
}

// Never send this to the other side.
const set<string> Mover::iReallyHave() const {
  return file_store()->availableHashes();
}

void Mover::handleExtension(MoverMessageProcessor *mmp, string data) {
  // It's safe to use UUID's here, even ones that might leak info
  // because the extension developers will either not care, or
  // will know enough to take precautions.
  // 36 is the length of an ascii uuid.
  if (data.length() < 36) {
    return;
  }
  UUID u = UUID(data.substr(0,36));
  AUTODEREF(MoverExtension *, mex);
  Synchronized(MoverExtension::extensionLock());
  mex = MoverExtension::lookup(u);
  if (mex != NULL) {
    mex->processIncoming(this, mmp, data);
  }
}

SecureMessageServer<MoverMessageProcessor> *Mover::connectToPeer(string peer) {
  SecureMessageServer<MoverMessageProcessor> *ss=NULL;
  AUTODEREF(SocketEventFactory *, sef);
  AUTODEREF(URL *, url);
  int ret;
  ss = new SecureMessageServer<MoverMessageProcessor>();

  HRNULL(ss);
  ss->initTLS(false);
  ss->setCiphers(SSL_TXT_HIGH);
  ss->set_coordinator(this);
  ss->useCertificate(cert());
  ss->setVerifier(verifier());
  url = URL::parseURL(peer);
  if (url->port() == 0) {
    url->setport(mover_default_port);
  }
  ss->connect(url);
  ss->ref();
 error_out:
  if (!ret) {
    return ss;
  } else {
    if (ss) {
      ss->deref();
    }
    return NULL;
  }
}

void Mover::pollPeers() {
  AUTODEREF(TextusFile *, fh);
  string *peers;

  // XXXXXX FXIME:  Is reseting this here revealing too much information?
  resetIHave(); // We will randomly change what we are willing to tell people we have whenever we poll.

  fh = TextusFile::openConfigFile(mover_peers_file, O_RDONLY);
  
  int range = mover_max_peers_connect - mover_min_peers_connect;
  int count = Random::rand(range) + mover_min_peers_connect;
  
  peers = Random::choose<TextusFile>(*fh, count);
  for (int i = 0; i < count; ++i) {
    if (peers[i].length() > 0) {
      AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, ss);
      ss = connectToPeer(peers[i]);
    }
  }

  delete [] peers;
}

// Called when we are trying to shut down, but someone
// still has a reference to us.
int Mover::weakDelete() {
  Synchronized(this);
  if (ss() != NULL) {
    ss()->close();
    setss(NULL);
  }
  setmmb(NULL);
  setcert(NULL);
  setmmf(NULL);
  seturl(NULL);
  sethasher(NULL);
  setverifier(NULL);
  setfile_store(NULL);
  if (timer() != NULL) {
    timer()->close();
  }
  settimer(NULL);
  return ServerCoordinator::weakDelete();
}

void Mover::startTimer() {
  Synchronized(this);
  if (!first_poll || mover_connect_immediately) {
    pollPeers();
  }
  while (Random::rand(mover_random_mail_probability) == 0) {
    createRandomMessage();
  }
  first_poll = false;
  AUTODEREF(MoverTimerEvent *, mte);
  mte = new MoverTimerEvent(Duration::seconds(Random::rand(mover_poll_time)),
			    this);
}

void Mover::timeOut() {
  startTimer();
}

void Mover::waitForShutdown() {
  Synchronized(this);
  while (!mover_done) {
    wait();
  }
}

int Mover::uploadFiles(list<string> files) {
  int ret = 0;
  int count = 0;
  AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, ss);
  AUTODEREF(MoverMessageProcessor *, mmp);
  ss = connectToPeer(mover_bind_address);
  HRC(ss->waitForConnect());
  mmp = ss->getProcessor();
  HRNULL(mmp);
  mmp->ref();
  for (list<string>::iterator i = files.begin(); i != files.end(); ++i) {
    AUTODEREF(TextusFile *, tf);
    tf = TextusFile::openFile(*i, O_RDONLY);
    if (tf == NULL) {
      LOG(INFO) << "Unable to open file " << *i << "\n";
      continue;
    }
    string data = tf->read(mover_max_data_size);
    Synchronized(mmp);
    mmp->sendMessage(data);
    count++;

  }
  if (mmp) {
    Synchronized(mmp);
    mmp->sendGoodBye();
  }

 error_out:
  if (ret == 0) {
    return count;
  }
  return -1;
}

int Mover::uploadData(string data) {
  int ret = 0;
  MoverMessageProcessor *mmp = NULL;
  AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, ss);
  ss = connectToPeer(mover_bind_address);
  HRC(ss->waitForConnect());
  mmp = ss->getProcessor();
  HRNULL(mmp);
  {
    Synchronized(mmp);
    mmp->sendMessage(data);
    mmp->sendGoodBye();
  }
 error_out:
  return ret;
}

string Mover::addHeader(uint64_t magic, string data, int type) {
  string out = Random::head_pad();
  out = Message::append(out, magic);
  out = Message::append(out, type);
  return out+data;
}

string Mover::getDataForHash(string hash) {
  return file_store()->getData(hash);
}

}} // namespace

