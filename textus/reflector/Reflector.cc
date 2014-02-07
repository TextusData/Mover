/* Reflector.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro 
 *
 * Implementation of a class to reflect connections.
 *
 */

#include "textus/base/Init.h"
#include "textus/reflector/Reflector.h"

DECLARE_INT_ARGUMENT(reflector_port, 4669, "reflector_port", "The port to listen on for new connections.");
DECLARE_STRING_ARGUMENT(reflector_certificate, "", "reflector_certificate",
			"The certificate path for the ssl connections.  "
			"Default is empty which means use a random self signed certificate.");

namespace textus { namespace reflector {

using namespace std;

int Reflector::bindCertificate(string certfile) {
}

}} // namespace

using namespace textus::reflector;
using namespace std;

int main(int argc, const char *argv[], const char *envp[])
{
  int ret;

  textus::base::init::TextusInit(argc, argv, envp);
  
  Reflector *reflector = new Reflector();
  if (!reflector) {
    fprintf (stderr, "Unable to create reflector object.\n");
    exit (1);
  }

  reflector->bindCertificate(reflector_certificate);
  reflector->bindServer(reflector_port);
  reflector->listen();


  textus::base::init::TextusWaitForExit();

  textus::base::init::TextusShutdown();

  return 0;
}

