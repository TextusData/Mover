/* SystemFile.h -*- c++ -*-
 * Copyright (c) 2009-2014 Ross Biro
 *
 * Represents a system file.
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

#ifndef TEXTUS_FILE_SYSTEM_FILE_H_
#define TEXTUS_FILE_SYSTEM_FILE_H_

#include "textus/file/FileHandle.h"

#include <string>

namespace textus { namespace file {

class SystemFile: public FileHandle {
public:
  SystemFile() { setNonBlocking(false); }
  virtual ~SystemFile();

  int open(const string filename, int access, int mode=0660);
  virtual int lock();
  virtual int unlock();
  virtual int sync();

  static std::string generateUniqueName();

};

}} //namespace

#endif // TEXTUS_FILE_FILE_H_
