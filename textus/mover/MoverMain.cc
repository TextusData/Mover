/* MoverMain.cc -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
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

#include "textus/mover/MoverMessageProcessor.h"
#include "textus/mover/Mover.h"
#include "textus/base/init/Init.h"
#include "textus/network/ssl/SecureMessageServer.h"
#include "textus/network/Socket.h"
#include "textus/mover/MoverEncryption.h"


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

  string config_path = mover_crypto_config;
  TextusFile *tf = TextusFile::openConfigFile(config_path, O_RDONLY);
  if (tf == NULL) {
    fprintf (stderr, "Unable to open crypto.cfg: %s\n", config_path.c_str());
    exit (3);
  }

  if (MoverEncryption::fromConfigFile(tf) != 0) {
    fprintf (stderr, "Unable to read crypto.cfg file: %s\n",
	     config_path.c_str());
    exit (2);
  }


  // If this is set, just upload files to localhost and bug out.
  mover->bindCertificate(mover_certificate);
  mv = new MoverVerifier();
  mover->setverifier(mv);
  if (mover_upload_files.size() != 0) {
    mover->uploadOnly();
    mover->uploadFiles(mover_upload_files);
    goto error_out;
  }

  HRC(mover->attachFileStore(textus::mover::mover_root));
  mover->resetIHave();

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
