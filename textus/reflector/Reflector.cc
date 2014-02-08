/* Reflector.cc -*- c++ -*-
 * Copyright (c) 2013 Ross Biro 
 *
 * Implementation of a class to reflect connections.
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

