/* MoverMain.cc -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
 *
 */

#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"
#include "textus/base/init/Init.h"
#include "textus/network/ssl/SecureMessageServer.h"
#include "textus/network/Socket.h"


using namespace textus::mover;
using namespace std;

int main(int argc, const char *argv[], const char *envp[])
{
  int ret = 0;
  AUTODEREF(MoverVerifier *, mv);

  // Default this to on so that we use TOR by default.
  textus::network::socket_use_socks = true;

  textus::base::init::TextusInit(argc, argv, envp);
  AUTODEREF(Mover *, mover);
  
  mover = new Mover();
  if (!mover) {
    fprintf (stderr, "Unable to create mover object.\n");
    exit (1);
  }


  // If this is set, just upload files to localhost and bug out.
  if (mover_upload_files.size() != 0) {
    mover->uploadOnly();
    mover->uploadFiles(mover_upload_files);
    goto error_out;
  }

  HRC(mover->attachFileStore(textus::mover::mover_root));
  mover->resetIHave();
  mover->bindCertificate(mover_certificate);
  mv = new MoverVerifier();
  mover->setverifier(mv);

  // bind causes the cerver to go active.
  mover->bindServer(mover_bind_address);
  mover->startTimer();

  if (mover_loopback_test) {
    AUTODEREF(SecureMessageServer<MoverMessageProcessor> *, ss);
    ss = mover->connectToPeer(string("localhost"));
  }

  if (mover_daemon) {
    mover->waitForShutdown();
  }

 error_out:
  textus::base::init::TextusShutdown();
  return ret;
}