/* OSSysInfo.cc -*- c++ -*-
 * Copyright (c) 2010, 2013, 2014 Ross Biro
 *
 * Linux version of SysInfo.h
 */

#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>

#include <string>

#include "textus/base/Base.h"
#include "textus/system/linux/OSSysInfo.h"
#include "textus/system/SysInfo.h"
#include "textus/file/FileHandle.h"
#include "textus/file/TextusFile.h"
#include "textus/file/LineReader.h"

namespace textus { namespace system {

using namespace std;

DEFINE_STRING_ARG(linux_random_device, "/dev/random", "linux_random_device", "The device used to get random numbers. (see also mixer.)");


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

