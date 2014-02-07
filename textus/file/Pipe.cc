/* Pipe.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a pipe for half duplex communications on a single system.
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
