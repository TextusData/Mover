/* SysRand.cc -*- c++ -*-
 * Copyright (c) 2013-2014 Ross Biro
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

#include <fcntl.h>

#include "textus/random/SysRand.h"
#include "textus/base/init/Init.h"
#include "textus/file/TextusFile.h"


namespace textus { namespace random {

using namespace std;
using namespace textus::file;

DEFINE_LIST_ARG_TEST(string, sysrand_paths, "sysrand_paths",
"The list of random number generator devices to use with sysrand.",
		     "/dev/urandom", "/dev/random");

void SysRand::openFiles() {
  Synchronized(this);
  file_handles.clear();
  for (std::list<std::string>::iterator i = sysrand_paths.begin();
       i != sysrand_paths.end(); ++i) {
    AUTODEREF(TextusFile *, fh);
    fh = TextusFile::openFile(*i, O_RDONLY);
    if (fh == NULL) {
      LOG(ERROR) << "Unable to open " << *i << " random number generation will suffer.";
    } else {
      file_handles.push_back(fh);
    }
  }
  assert(file_handles.size() > 0);
}

void SysRand::reloadBuffer() {
  Synchronized(this);
  if (file_handles.size() == 0) {
    openFiles();
  }

  /* save the waisted entropy. */
  for (unsigned i = sizeof(buffer)-1; i > buff_ptr; --i) {
    buffer[sizeof(buffer) - 1 - i] ^= buffer[i];
  }
  buff_ptr = 0;

  for (ReferenceList<TextusFile *>::iterator i = file_handles.begin();
       i != file_handles.end(); ++i) {
    string d = (*i)->readAll(sizeof(buffer));
    if (d.length() != sizeof(buffer)) {
      LOG(ERROR) << "Unable to read system random device.";
      die();
    }
    for (unsigned i = 0; i < sizeof(buffer); ++i) {
      buffer[i] ^= d[i];
    }
  }
}

uint64_t SysRand::randombits(int bits) {
  volatile uint64_t r;
  assert (bits > 0 && bits <=64);
  int bytes = (bits + 7)/8;
  Synchronized(this);
  if (buff_ptr + bytes >= sizeof(buffer)) {
    reloadBuffer();
  }
  for (int i = 0; i < bytes; ++i) {
    r = (r >> 56) | r << 8;
    r = r ^ buffer[buff_ptr + i];
  }
  buff_ptr += bytes;
  return r;
}



}} //namespace
