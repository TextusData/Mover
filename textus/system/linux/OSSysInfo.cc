/* OSSysInfo.cc -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * Linux version of SysInfo.h
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

#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>

#include <string>
#include <set>

#include "textus/base/Base.h"
#include "textus/system/linux/OSSysInfo.h"
#include "textus/system/SysInfo.h"
#include "textus/file/FileHandle.h"
#include "textus/file/TextusFile.h"
#include "textus/file/LineReader.h"

namespace textus { namespace system {

using namespace std;

DEFINE_STRING_ARG(linux_random_device, "/dev/random", "linux_random_device", "The device used to get random numbers. (see also mixer.)");

static set<string> cpu_flags;

bool SysInfo::getCPUFlag(std::string f) {
  return cpu_flags.count(f) != 0;
}

void saveSystemFlags(string flags) {
  size_t off = 0;
  size_t end = 0;
  string flag;
  while (true) {
    off = flags.find_first_not_of(" \t\r\n", end);
    if (off == string::npos) {
      return;
    }
    end = flags.find_first_of(" \t\r\n", off);
    if (end == string::npos) {
      cpu_flags.insert(flags.substr(off));
      return;
    }
    cpu_flags.insert(flags.substr(off, end - off));
  }
}

int SysInfo::getNumCPUs() {
  AUTODEREF(textus::file::FileHandle *, fh);
  fh = dynamic_cast<textus::file::FileHandle *>(textus::file::TextusFile::openFile ("/proc/cpuinfo", O_RDONLY));
  AUTODEREF(textus::file::LineReader *, lr);
  lr = new textus::file::LineReader(fh);
  int cpu_count = 0;
  while (!lr->eof()) {
    string s = lr->waitForLine();
    if (s.substr(0, 11) == "processor\t:") {
      cpu_count++;
    }
    if (s.substr(0,5) == "flags") {
      size_t colon = s.find(':');
      if (colon != string::npos) {
	saveSystemFlags(s.substr(colon));
      }
    }
  }
  return cpu_count;
}

std::string SysInfo::getAppPath() {
  return string("");
}

void SysInfo::random(unsigned char *buff, unsigned len) {
  EXEC_BARRIER();
  // XXXXX FIXME: cache the file handle, we will likely need it alot.
  // XXXXX FIXME: we also need to cache some random data and 
  //              put some noise into the reads.  We want to
  //              avoid giving something away by our pattern of
  //              reading /dev/random.
  // XXXXXX FIXME: switch this back to /dev/random before production.
  int fd = open(linux_random_device.c_str(), O_RDONLY); 
  if (fd >= 0) {
    read (fd, buff, len);
    close(fd);
  }
}

string SysInfo::getUser() {
  uid_t u = getuid();
  struct passwd pwd;
  char buff[1024]; // XXXXXX FIXME, we should make this variable and retry.
  struct passwd *res;

  if (getpwuid_r(u, &pwd, buff, sizeof(buff), &res) != 0 || res == NULL) {
    return "";
  }
  
  return string(res->pw_name);
}

}} //namespace

