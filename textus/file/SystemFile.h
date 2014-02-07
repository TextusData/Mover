/* SystemFile.h -*- c++ -*-
 * Copyright (c) 2009-2014 Ross Biro
 *
 * Represents a system file.
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
