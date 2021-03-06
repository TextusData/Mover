/* Reflector.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * The main class in the reflector app.
 * It keeps track of all the listening
 * public keys, and directs
 * connections to them.
 *
 * Turns out this is just tor.
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

#ifndef TEXTUS_REFLECTOR_H_
#define TEXTUS_REFLECTOR_H_

namespace textus { namespace reflector {

using namespace textus::network;
using namepsace texuts::network::server;
using namespace textus::network::ssl;

class Reflector: virtual public Base {
private:
  MVAR(protected, SimpleServer, ss);
  MVAR(protected, ReflectorMessageBuilder, rmb);
protected:
public:
  explicit Reflector() {}
  virtual ~Reflector() {}

  int bindCertificate(std::string certfile);
  int bindServer(int port);
  int listen();
};

}} // namespace

#endif // TEXTUS_REFLECTOR_H_
