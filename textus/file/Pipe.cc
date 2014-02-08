/* Pipe.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a pipe for half duplex communications on a single system.
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

#include <unistd.h>
#include "textus/file/Pipe.h"

namespace textus { namespace file {

Pipe::Pipe() {
  int fds[2];

  AUTODEREF(FileHandle *, wh);
  AUTODEREF(FileHandle *, rh);
  wh = new FileHandle();
  rh = new FileHandle();

  writerHandle = wh;
  readerHandle = rh;

  EXEC_BARRIER();
  if (pipe(fds) < 0 || fds[0] < 0 || fds[1] < 0) {
    LOG(ERROR) << "Pipe failed to create pipes: "<<errno << "\n";
    die();
  }

  wh->open(fds[1]);
  rh->open(fds[0]);

}

}} //namespace
