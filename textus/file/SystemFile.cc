/* SystemFile.cc -*- c++ -*-
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

#include "textus/file/SystemFile.h"
#include "textus/base/UUID.h"
#include <sys/time.h>
#include <sys/file.h>

namespace textus { namespace file {

SystemFile::~SystemFile()
{
  // Nothing to clean up at the moment.
}

int SystemFile::sync() {
  return fsync(nativeHandle());
}

int SystemFile::open(const string filename, int access, int mode)
{
  EXEC_BARRIER();
  int fd = ::open (filename.c_str(), access, mode);
  if (fd < 0) 
    return fd;

  FileHandle::open(fd);
  return fd;
}

int SystemFile::lock() {
  return flock(nativeHandle(), LOCK_EX);
}

int SystemFile::unlock() {
  return flock(nativeHandle(), LOCK_UN);
}

using namespace std;

string SystemFile::generateUniqueName() {
  // Need to identify the machine as well.
  int pid = getpid();
  struct timeval now;
  gettimeofday(&now, NULL);
  char buf[40];
  snprintf (buf, sizeof(buf), "_%d_%ld.%06ld", pid, (long)now.tv_sec, (long)now.tv_usec);
  string fn = base::init::getAppName(); // some information that might
					// be useful for
					// debugging. Need to include
					// something to identify the
					// machine.
  fn += "_";
  fn += buf;
  fn += "_";
  UUID *uuid = new UUID();
  if (uuid) {
    uuid->generate();
    fn += uuid->toString();
    delete uuid;
  }
  return fn;
}

}} // namespace
