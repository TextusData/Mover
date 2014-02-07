/* SecureMessageServer.h -*- c++ -*-
 * Copyright (c) 2010, 2013 Ross Biro
 *
 * Class for a simple message oriented server.
 *
 */

#ifndef TEXTUS_NETWORK_SSL_SECURE_MESSAGE_SERVER_H_
#define TEXTUS_NETWORK_SSL_SECURE_MESSAGE_SERVER_H_

#include "textus/base/Base.h"
#include "textus/network/ssl/SecureSocket.h"
#include "textus/file/ProtoBufReaderEvent.h"
#include "textus/network/server/ServerTemplate.h"

namespace textus { namespace network { namespace ssl {

using namespace textus::network::server;

template <class T> class SecureMessageServer: 
    public ServerSocket<T, SecureSocket, textus::file::ProtoBufReader> 
{
private:

public:
  explicit SecureMessageServer() {
    ServerSocket<T, SecureSocket, ProtoBufReader>::lr = new ProtoBufReader(this);
    ServerSocket<T, SecureSocket, ProtoBufReader>::lr->deref();

    AUTODEREF(textus::file::ProtoBufReader::EventFactoryType *, lref);
    lref = new textus::file::ProtoBufReader::EventFactoryType(ServerSocket<T, SecureSocket, ProtoBufReader>::lr);
    ServerSocket<T, SecureSocket, ProtoBufReader>::lr->setEventFactory(lref);

    ServerSocket<T, SecureSocket, ProtoBufReader>::processor = new T(this);
    ServerSocket<T, SecureSocket, ProtoBufReader>::processor->deref();

  }

  virtual ~SecureMessageServer() {}

  ServerCoordinator *coordinator() {
    return ServerSocket<T, SecureSocket, ProtoBufReader>::processor->coordinator();
  }

  void setcoordinator(ServerCoordinator *c) {
    ServerSocket<T, SecureSocket, ProtoBufReader>::processor->setcoordinator(c);
  }

  T *getProcessor() {
    return ServerSocket<T, SecureSocket, ProtoBufReader>::processor;
  }
};


template <class T> class SecureMessageServerFactory: public SecureSocketFactory
{
private:
  string ciphers;
  bool tls;
  bool useCertFile;
  AutoDeref<Certificate> cert_;
  MVAR(public, ServerCoordinator, coordinator);
  MVAR(public, SSLVerifier, verifier);

public:
  SecureMessageServerFactory(): tls(false), useCertFile(true) {}
  virtual ~SecureMessageServerFactory() {}

  void setcert(Certificate *c) {
    Synchronized(this);
    useCertFile = false;
    cert_ = c;
  }

  Certificate *cert() {
    Synchronized(this);
    return cert_;
  }

  virtual SecureSocket *createSecureSocket()
  {
    SecureMessageServer<T> *s = new SecureMessageServer<T>();
    Synchronized(this);
    Synchronized(s);
    if (tls) {
      s->initTLS(useCertFile);
    } else {
      s->init(useCertFile);
    }

    if (ciphers.length() > 0) {
      s->setCiphers(ciphers.c_str());
    }

    s->setcoordinator(coordinator());
    if (!useCertFile) {
      s->useCertificate(cert());
    }
    if (verifier() != NULL) {
      s->setVerifier(verifier());
    }
    return s;
  }

  void setCiphers(std::string c) {
    Synchronized(this);
    ciphers = c;
  }

  void setTLS(bool t) {
    Synchronized(this);
    tls = t;
  }

  bool getTLS() {
    Synchronized(this);
    return tls;
  }

};

}}} //namespace


#endif // TEXTUS_NETWORK_SSL_SECURE_MESSAGE_SERVER_H_
