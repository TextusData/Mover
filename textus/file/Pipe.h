/* Pipe.h -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Represents a pipe for half duplex communications on a single system.
 *
 */

#ifndef TEXTUS_FILE_PIPE_H_
#define TEXTUS_FILE_PIPE_H_

#include "textus/base/Base.h"
#include "textus/file/FileHandle.h"

namespace textus { namespace file {

class Pipe: public Base {
private:
  AutoDeref<FileHandle> readerHandle;
  AutoDeref<FileHandle> writerHandle;

public:
  Pipe();
  virtual ~Pipe() {}
  
  FileHandle *reader() { return readerHandle; }
  FileHandle *writer() { return writerHandle; }
};

}} //namespace

#endif // TEXTUS_FILE_PIPE_H_
