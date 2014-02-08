/* Pipe.h -*- c++ -*-
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
