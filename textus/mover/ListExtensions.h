/* ListExtensions.h -*- c++ -*-
 * Copyright (c) 2013 Ross Biro
 *
 * A Mover Extension that lists the installed extensions.
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

#ifndef TEXTUS_MOVER_LIST_EXTENSIONS_H_
#define TEXTUS_MOVER_LIST_EXTENSIONS_H_

#include "MoverExtension.h"

namespace textus { namespace mover {

class ListExtensionsRequest: public MoverExtension {
private:
public:
  ListExtensionsRequest() {}
  virtual ~ListExtensionsRequest() {}
  virtual void processIncoming(Mover *, MoverMessageProcessor *, std::string);
};

class ListExtensionsReply: public MoverExtension {
private:
public:
  ListExtensionsReply() {}
  virtual ~ListExtensionsReply() {}
  virtual void processIncoming(Mover *, MoverMessageProcessor *, std::string);
};

}} //namespace

#endif // TEXTUS_MOVER_LIST_EXTENSIONS_H_
