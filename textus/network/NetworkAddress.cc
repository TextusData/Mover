/* NetworkAddress.cc -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * Represents a network address.
 *
 */

#include "textus/network/NetworkAddress.h"
#include "textus/network/Socket.h"
#include "textus/network/UDPSocket.h"
#include "textus/base/ReferenceValueCache.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/random/PRNG.h"
#include "textus/base/init/Init.h"
#include "textus/file/LineReader.h"

#include <arpa/inet.h>

#define DNS_PORT 53

namespace textus { namespace network {

DEFINE_INT_ARG(network_address_max_ttl, 3600, "network-address-max-ttl",
	       "The maximum amount of time to keep a network address around.");


using namespace textus::file;

struct __attribute__((packed)) DNSRequestHeader {
  enum opcodes {
    query = 0,
    iquery = 1,
    status = 2,
  };

  enum rcode {
    ok = 0,
    format_error = 1,
    server_failure = 2,
    name_error = 3,
    not_implemented = 4,
    refused = 5,
  };
  /*header */
  uint16_t id;
  uint16_t rd:1, tc:1, aa:1, opcode:4, qr:1, rcode:4, cd:1, ad:1, z:1, ra:1;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;

  /* The rest varies depending on the header. */
};

class Resolver;
class ResolverSocketEvent;
class ResolverSocketEventFactory;

class ResolverSocket: virtual public Base {
private:
  AutoDeref<UDPSocket> s;
  AutoDeref<NetworkAddress> server;

protected:
  int weakDelete() {
    Synchronized(this);
    if (s) {
      s->setEventFactory(NULL);
    }
    return Base::weakDelete();
  }
public:
  ResolverSocket() {}
  virtual ~ResolverSocket() {}
  int Init();

  void sendPacket(Resolver *r, void *pack, size_t len) {
    string d(static_cast<char *>(pack), len);
    s->sendData(d);
  }

  NetworkAddress *networkAddress() {
    Synchronized(this);
    return server;
  }

  void setNetworkAddress(NetworkAddress *s) {
    Synchronized(this);
    server = s;
  }

  static NetworkAddress *getDNSServer();

  void error(ResolverSocketEvent *);
  void read(ResolverSocketEvent *);
  void canWrite(ResolverSocketEvent *);

  Socket *socket() {
    Synchronized(this);
    return s;
  }
};

class ResolverSocketEventFactory: public UDPSocketEventFactory {
private:
  AutoWeakDeref<ResolverSocket> parent;
  
public:
  ResolverSocketEventFactory(ResolverSocket *p):UDPSocketEventFactory(p->socket()), parent(p) {}
  virtual ~ResolverSocketEventFactory() {}
  
  ResolverSocket *resolverSocket() {
    Synchronized(this);
    return parent;
  }

  void setResolverSocket(ResolverSocket *rs) {
    Synchronized(this);
    parent = rs;
  }

  virtual textus::event::Event *errorEvent(Error *e);
  virtual textus::event::Event *readEvent(string s, NetworkAddress *src);
  virtual textus::event::Event *writeEvent();

};

class ResolverSocketEvent: public SocketEvent {
private:
  AutoDeref<ResolverSocket> parent;
  AutoDeref<Error> err;
  string dat;
  AutoDeref<NetworkAddress> src;
  bool write;

protected:
  virtual void do_it();

public:
  ResolverSocketEvent(ResolverSocket *rs): SocketEvent(rs->socket()), parent(rs) {}
  virtual ~ResolverSocketEvent() {}

  ResolverSocket *resolverSocket() {
    Synchronized(this);
    return parent;
  }

  void setResolverSocket(ResolverSocket *rs) {
    Synchronized(this);
    parent = rs;
  }

  string data() {
    Synchronized(this);
    return dat;
  }

  void setData(string d) {
    Synchronized(this);
    dat = d;
  }

  Error *error() {
    Synchronized(this);
    return err;
  }

  void setError(Error *e) {
    Synchronized(this);
    err = e;
  }

  NetworkAddress *srcNetworkAddress() {
    Synchronized(this);
    return src;
  }

  void setSrcNetworkAddress(NetworkAddress *na) {
    Synchronized(this);
    src = na;
  }

  bool canWrite() {
    Synchronized(this);
    return write;
  }

  void setCanWrite(bool t) {
    Synchronized(this);
    write = t;
  }
  
};

class Resolver: virtual public Base {
public:
  enum error_types {
    NO_ERROR = 0,
    E_BAD_HOSTNAME=65536,
    E_TO_LONG,
    E_NO_SERVER,
  };


private:
  int error;
  uint16_t requestID;

  enum ResponseType {
    A = 1,
    NS = 2,
    MD = 3,
    MF = 4,
    CNAME = 5,
    SOA = 6,
    MB = 7,
    MG = 8,
    MR = 9,
    NLL = 10,
    WKS = 11,
    PTR = 12,
    HINFO = 13,
    MINFO = 15,
    MX = 16,
    TXT = 17,
  };

  enum ClassType {
    IN = 1, 
    CS = 2,
    CH = 3,
    HS = 4,
  };

  AutoDeref<NetworkAddress> na;

protected:
  ResolverSocket *getSocket();

  const char *eatQuery(const char *, const char *, size_t) const;
  const char *processAnswer(const char *, const char *, size_t);
  void queryDone();
  void setAddress(string);
public:

  Resolver() {}
  virtual ~Resolver() {}
  size_t computeLabelLength(string s);
  void start();
  void processResponse(string buffer);
  void setNetworkAddress(NetworkAddress *n) {
    Synchronized(this);
    na = n;
  }

  void packetDone() {
    if (!na->empty()) {
      na->setValid(true);
    }
  }

  void clearAddresses() {
    na->setValid(false);
    na->clearAddresses();
  }
  
  void addAddress4(string d, unsigned long ttl) {
    na->addAddress4(d);
    if (ttl > (unsigned) network_address_max_ttl) {
      ttl = network_address_max_ttl;
    }
    Time t = Time::now() + Duration::seconds(ttl);
    if (t == Time((time_t)0)) {
      t = Time((time_t)-1);
    }
    if (na->expiresTime() == Time((time_t)0) || na->expiresTime() > t) {
      na->setExpiresTime(t);
    }
  }


};

/***** Static Defines that depend on the classes above. *****/
static ReferenceValueMap<uint16_t, Resolver *> request_id_map;
static ReferenceValueCache<string, NetworkAddress *> addressCache;

textus::event::Event *ResolverSocketEventFactory::errorEvent(Error *e) {
  ResolverSocketEvent *rse = new ResolverSocketEvent(parent);
  rse->setError(e);
  setEventQueue(rse);
  return rse;
}

textus::event::Event *ResolverSocketEventFactory::writeEvent() {
  ResolverSocketEvent *rse = new ResolverSocketEvent(parent);
  rse->setCanWrite(true);
  setEventQueue(rse);
  return rse;
}

textus::event::Event *ResolverSocketEventFactory::readEvent(string data, NetworkAddress *src) {
  ResolverSocketEvent *rse = new ResolverSocketEvent(parent);
  rse->setData(data);
  rse->setSrcNetworkAddress(src);
  setEventQueue(rse);
  return rse;
}

void ResolverSocketEvent::do_it() {
  if (parent) {
    if (error()) {
      parent->error(this);
    }
    if (data().length()) {
      parent->read(this);
    }
    if (canWrite()) {
      parent->canWrite(this);
    }
  }
}

void ResolverSocket::error(ResolverSocketEvent *rse) {
  LOG(ERROR) << rse->error();
}

void ResolverSocket::read(ResolverSocketEvent *rse) {
#if 0
  // XXXXXXXXXXXXXXXXXXXXX
  // This isn't working on Linux.  It looks like the
  // address is getting corrupted.
  if (*rse->srcNetworkAddress() != *server) {
    LOG(ERROR) << "DNS Response from wrong server: " << *server << "\n";
    return;
  }
#endif

  const struct DNSRequestHeader *header = reinterpret_cast<const struct DNSRequestHeader *>(rse->data().c_str());
  uint16_t id = header->id;

  assert(!textus::base::init::shutdown_done);
  Synchronized(&request_id_map);
  AUTODEREF(Resolver *, res);
  ReferenceValueMap<uint16_t, Resolver *>::iterator i = request_id_map.find(id);
  if (i == request_id_map.end()) {
    // probably just late.  We need to ignore it though incase it's an
    // attempt at cache poisoning.
    LOG(WARNING) << "DNS response for unknown request.\n";
    return;
  }
  res = i->second;
  res->ref();
  request_id_map.erase(i);
  res->processResponse(rse->data());
}

void ResolverSocket::canWrite(ResolverSocketEvent *) {
  /* Don't need to do anything.  We currently don't throttle. */
}

int ResolverSocket::Init() {
  AUTODEREF(UDPSocket *, sock);
  sock = new UDPSocket();
  s = sock;

  AUTODEREF(ResolverSocketEventFactory *, rsef);
  rsef = new ResolverSocketEventFactory(this);

  s->setEventFactory(rsef);

  server = getDNSServer();
  if (server == NULL) {
    return -1;
  }
  s->connect(server);

  return 0;
}

class ServerName: virtual public Base {
private:
  AutoDeref<NetworkAddress> na;
  Time next_use;

public:
  ServerName():next_use(Time::now()) {}
  virtual ~ServerName() {}

  void setAddress(NetworkAddress *n) {
    Synchronized(this);
    na = n;
  }

  NetworkAddress *address() {
    Synchronized(this);
    return na;
  }

  void setNextUse(Time t) {
    Synchronized(this);
    next_use = t;
  }

  Time netUse() {
    Synchronized(this);
    return next_use;
  }

  bool valid() {
    Synchronized(this);
    if (next_use < Time::now())
      return true;
    return false;
  }
  
};

DEFINE_STRING_ARG(resolv_conf, "/etc/resolv.conf", "resolve_conf", "Name of file to get list of nameservers from.");
static ReferenceList<ServerName *> name_servers;

NetworkAddress *ResolverSocket::getDNSServer()
{
  assert(!textus::base::init::shutdown_done);
  Synchronized(&name_servers);
  if (name_servers.empty()) {
    AUTODEREF(FileHandle *, fh);
    fh = textus::file::FileHandle::openFile(resolv_conf, O_RDONLY);
    if (fh == NULL) {
      return NULL;
    }
    AUTODEREF(LineReader *, lr);
    lr = new LineReader(fh);
    while (!lr->eof()) {
      string d = lr->waitForLine();
      string::size_type n = d.find_first_not_of(" \t\r\n");
      if (n != string::npos && d.substr(n,10) == "nameserver") {
	string::size_type m = d.find_first_not_of(" \t\r\n", n+10);
	if (m != string::npos) {
	  n = d.find_first_of(" \t\r\n", m);
	  if (n == string::npos) {
	    n = d.length();
	  }
	  AUTODEREF(ServerName *, sn);
	  AUTODEREF(NetworkAddress *, na);
	  na = NetworkAddress::addressFromString(d.substr(m, n-m));
	  if (na == NULL) {
	    continue;
	  }
	  na->setPort(htons(DNS_PORT));
	  sn = new ServerName();
	  sn->setAddress(na);
	  name_servers.push_back(sn);
	}
      }
    }
  }
  if (name_servers.empty()) {
    return NULL;
  }
  ServerName *first = name_servers.front();
  ServerName *sn = first;
  do {
    sn->ref();
    name_servers.pop_front();
    name_servers.push_back(sn);
    sn->deref();
    if (sn->valid()) {
      return sn->address();
    }
    sn = name_servers.front();
  } while (first != sn);
  return first->address();
}

size_t Resolver::computeLabelLength(string s) {
  // length is the same as string plus the null terminator plus 1.
  // the dots are replace by length bytes, but we need one extra one
  // however, if it ends with a dot, it doesn't count.
  size_t sz = 0;
  size_t p = 0;
  size_t l = s.length();
  while (p < l) {
    while (s[p++] == '.' && p <= l);
    p--;
    if (p < l) {
      sz++;
    }
    while (s[p++] != '.' && p <= l) {
      sz++;
    }
  }
  return sz+1;
}

void Resolver::start() {
  size_t len = computeLabelLength(na->hostname());
  len = len + 4; // QType and QClass.
  len = len + sizeof(struct DNSRequestHeader);

  void *packet = alloca(len);
  memset(packet, 0, len);
  struct DNSRequestHeader *rh = reinterpret_cast<struct DNSRequestHeader *>(packet);
  rh->id = htons(textus::random::PRNG::globalPRNG()->rand16());
  requestID = rh->id;
  request_id_map[rh->id] = this;
  rh->qr = 0;
  rh->opcode = DNSRequestHeader::query;
  rh->aa = 0;
  rh->tc = 0;
  rh->rd = 1;
  rh->ra = 0;
  rh->z = 0;
  rh->rcode = DNSRequestHeader::ok;
  rh->qdcount = htons(1);
  rh->ancount = htons(0);
  rh->nscount = htons(0);
  rh->arcount = htons(0);

  char *cptr = reinterpret_cast<char *>(rh + 1);
  
  std::string::size_type start = 0;
  std::string::size_type p = na->hostname().find('.', start); 
  if (p == 0) {
    LOG(WARNING) << "Bad hostname (" << na->hostname() << ") in Resolver.\n";
    error = E_BAD_HOSTNAME;
    queryDone();
    return;
  }
  do {
    std::string::size_type len;
    if (p == std::string::npos) {
      len = na->hostname().length() - start;
    } else {
      len = p - start;
    }
    if (len >= 64) {
      LOG(ERROR) << "name to long for dns\n";
      error = E_TO_LONG;
      queryDone();
      return;
    }
    if (len == 0) {
      break;
    }
    *reinterpret_cast<unsigned char *>(cptr++) = len;
    memcpy(cptr, na->hostname().c_str() + start, len);
    cptr += len;
    start += len+1;
    p = na->hostname().find('.', start); 
    if (p == string::npos) {
      p = na->hostname().length();
    }
  } while (start < na->hostname().length());
  *cptr++ = 0;
  *reinterpret_cast<unsigned char *>(cptr++) = A >> 8;
  *reinterpret_cast<unsigned char *>(cptr++) = A & 0xFF;
  *reinterpret_cast<unsigned char *>(cptr++) = 0;
  *reinterpret_cast<unsigned char *>(cptr++) = IN;

  AUTODEREF(ResolverSocket *, rs);
  rs = getSocket();
  if (rs == NULL) {
    return;
  }
  Synchronized(rs);
  
  rs->sendPacket(this, packet, len);
}

void Resolver::processResponse(string data) 
{
  if (data.length() < sizeof(struct DNSRequestHeader)) {
    LOG(WARNING) << "Short response from server.\n";
  }
  const struct DNSRequestHeader *header = reinterpret_cast<const struct DNSRequestHeader *>(data.c_str());
  if (header->id != requestID) {
    LOG(ERROR) << "Response with wrong ID in process response.\n";
    // Don't know how we could get here.
    return;
  }
  const char *cptr = reinterpret_cast<const char *>(header + 1);
  if (header->rcode != DNSRequestHeader::ok) {
    LOG(INFO) << "DNS error resopnse for " << na->hostname() << " " << header->rcode << ".\n";
    if (header->rcode == DNSRequestHeader::format_error) {
      LOG(ERROR) << "DNS error Fromat Error.\n";
    }
    error = header->rcode;
    na->setError(error);
    return;
  }

  clearAddresses();

  for (int i = 0; i < ntohs(header->qdcount); i++) {
    cptr = eatQuery(reinterpret_cast<const char *>(header), cptr, data.length());
    if (cptr == NULL)
      return;
  }
  
  for (int i = 0; i < ntohs(header->ancount) + ntohs(header->nscount) + ntohs(header->arcount); i++) {
    cptr = processAnswer(reinterpret_cast<const char *>(header), cptr, data.length());
    if (cptr == NULL)
      return;
  }

  packetDone();

}

const char *Resolver::eatQuery(const char *base, const char *cptr, size_t length) const
{
  while (*cptr != 0 && (*cptr & 0xC0) != 0xC0) {
    cptr = cptr + *cptr + 1;
    if (cptr > base + length) {
      LOG(WARNING) << "Short Packet in eatQuery.\n";
      return NULL;
    }
  }
  cptr +=5;
  return cptr;
}


/* XXXXXXXXXXXXX FIXME: BUG BUG BUG bounds checking missing!!! BUG BUG BUG XXXXXXXXXXXXXXXXXXX */
const char *Resolver::processAnswer(const char *base, const char *cptr, size_t length)
{
  string name;
  const char *next = NULL;
  while (*cptr != 0) {
    if((*cptr & 0xC0) == 0xC0) {
      cptr++;
      if (next == NULL)
	next = cptr;
      cptr = base + *cptr;
      continue;
    } 

    if (name.length() > 0) {
      name = name + ".";
    }
    name = name + string(cptr+1, *cptr);
    cptr = cptr + *cptr + 1;
  }
  if (next == NULL) 
    next = cptr;

  cptr = next + 1;
  uint16_t type = *cptr++ << 8;
  type |= *cptr++;
  uint16_t cls =  *cptr++ << 8;
  cls |= *cptr ++;
  int32_t ttl = *cptr++ << 24;
  ttl |= *cptr++ << 16;
  ttl |= *cptr++ << 8;
  ttl |= *cptr++;
  uint16_t rdlen = *cptr++ << 8;
  rdlen |= *cptr ++;
  const char *rdata = cptr;
  cptr += rdlen;
  if (type == A && cls == IN) {
    string a(rdata, rdlen);
    addAddress4(a, ttl);
  }
  return cptr;
}

static ReferenceList<ResolverSocket *> resolverSocketList;

ResolverSocket *Resolver::getSocket() {
  assert(!textus::base::init::shutdown_done);
  Synchronized(&resolverSocketList);
  if (resolverSocketList.empty()) {
    AUTODEREF(ResolverSocket *, rs);
    rs = new ResolverSocket();
    if (rs->Init() < 0) {
      error = E_NO_SERVER;
      queryDone();
      return NULL;
    }
    resolverSocketList.push_front(rs);
  }
  ResolverSocket *rs = resolverSocketList.front();
  rs->ref();
  resolverSocketList.pop_front();
  resolverSocketList.push_back(rs);
  return rs;
}

void Resolver::queryDone() {
  Synchronized(this);
  na->setValid(true);
  na->signal();
}

using namespace std;
NetworkAddress *NetworkAddress::resolve(string hostname)
{
  assert(!textus::base::init::shutdown_done);
  Synchronized(&addressCache); // guarantees the address will stay valid through the ref.
  ReferenceValueCache<string, NetworkAddress *>::iterator i = addressCache.find(hostname);
  if (i != addressCache.end()) {
    NetworkAddress *na = i->second;
    if (!na->expired()) {
      na->ref();
      return na;
    }
    // may invalidate na at this point.
    // Don't try to reuse the old one because
    // others may be using it not knowing it has
    // expired, or it may have just expired on them.
    addressCache.erase(i);
    na = NULL;
  }

  NetworkAddress *na = new NetworkAddress();
  na->setHostname(hostname);
  addressCache[hostname] = na;
  na->startResolve();

  return na;
}

void NetworkAddress::startResolve() {
  Resolver *r = new Resolver();
  r->setNetworkAddress(this);
  r->start();
  r->deref();
}

NetworkAddress *NetworkAddress::addressFromString(string s) {
  void *buffer = alloca(sizeof(in6_addr));
  
  if (inet_pton(AF_INET, s.c_str(), buffer) == 1) {
    NetworkAddress *na = new NetworkAddress(AF_INET, *reinterpret_cast<in_addr_t *>(buffer), 0);
    return na;
  }

  //XXXXXXXXX put in ipv6 stuff here when we support it.

  return NULL;
  
}

DEFINE_INIT_FUNCTION(resolverInit, NETWORK_INIT_PRIORITY - 1) {
  textus::random::PRNG::globalPRNG()->seed();
  return 0;
}

DEFINE_SHUTDOWN_FUNCTION(resolverShutdown, THREAD_SHUTDOWN_PRIORITY + 1) {
  addressCache.clear();
  request_id_map.clear();
  name_servers.clear();
  resolverSocketList.clear();
}


}} //namespace
