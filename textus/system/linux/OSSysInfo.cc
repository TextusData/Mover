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
#include "textus/threads/WorkerThread.h"

namespace textus { namespace system {

using namespace std;

// XXXXXX FIXME: switch this back to /dev/random before production.
DEFINE_STRING_ARG(linux_random_device, "/dev/urandom", "linux_random_device", "The device used to get random numbers. (see also mixer.)");

static set<string> cpu_flags;


bool SysInfo::getCPUFlag(std::string f) {
  getNumCPUs(); // need to make sure the flags are already set.
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
  static int num_cpus = -1;
  static textus::base::Base lock;
  Synchronized(&lock);
  if (num_cpus > 0) {
    return num_cpus;
  }

  // Make sure we have at least one worker thread.
  textus::threads::WorkerThread::getFirst();

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
  num_cpus = cpu_count;
  return num_cpus;
}

std::string SysInfo::getAppPath() {
  return string("");
}

void SysInfo::random(unsigned char *obuff, unsigned len) {
  EXEC_BARRIER();
  // XXXXX FIXME: we also need to cache some random data and 
  //              put some noise into the reads.  We want to
  //              avoid giving something away by our pattern of
  //              reading /dev/random.
  unsigned char *ibuff = static_cast<unsigned char *>(alloca(len));
  static int fd=-1;
  static textus::base::Base lock;
  {
    Synchronized(&lock);
    if (fd < 0) {
      fd = open(linux_random_device.c_str(), O_RDONLY); 
    }
  }
  if (fd >= 0) {
    read (fd, ibuff, len);
  } else {
    LOG(ERROR) << "Unable to open random device.\n";
    die();
  }
  for (unsigned i = 0; i < len; i++) {
    obuff[i] ^= ibuff[i];
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

