/* OSSysInfo.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * OS-X version of SysInfo.h
 */

#include "textus/base/Base.h"
#include "textus/system/darwin/OSSysInfo.h"
#include "textus/system/SysInfo.h"
#include "textus/file/Shell.h"
#include "textus/base/ThreadLocal.h"

#include <string>

namespace textus { namespace system {

using namespace std;
using namespace base;

int SysInfo::getNumCPUs() {
  AUTODEREF(textus::file::Shell *, shell);
  shell = new textus::file::Shell();
  shell->setCommand("hostinfo");
  shell->go();
  shell->waitForCompletion();
  string output = shell->getOutput();
  size_t start = output.find("Processors active:");
  if (start == string::npos) {
    LOG(ERROR) << "Unable to find number of processors in hostinfo output.\n";
    die();
  }
  start = start + 18;
  size_t end = output.find_first_of("\n\r", start);
  if (end == string::npos) {
    LOG(ERROR) << "Unable to find number of processors in hostinfo output.\n";
    die();
  }
  string cpus = output.substr(start, end-start);
  int count = 0;
  while ((start = cpus.find_first_of("\t ")) != string::npos) {
    count++;
    cpus = cpus.substr(start+1);
  }

  return count;
}

std::string SysInfo::getAppPath() {
  return string("");
}

void SysInfo::random(unsigned char *obuff, unsigned len) {
  EXEC_BARRIER();
  unsigned char *ibuff = static_cast<unsigned char *>(alloca(len));
  int fd = open("/dev/srandom", O_RDONLY);
  if (fd >= 0) {
    read (fd, ibuff, len);
    close(fd);
    for (unsigned i = 0; i < len; i++) {
      obuff[i] ^= ibuff[i];
    }
  }
}

static ThreadLocal<bool> validAddress;

void addressValidSigAction(int sig) {
  validAddress = false;
}

bool SysInfo::addressValid(void *vptr) {
  struct sigaction sa;
  struct sigaction osa;
  bool b;
  b = true;
  validAddress = b;

  sa.sa_handler = addressValidSigAction;
  sa.sa_mask = 0;
  sa.sa_flags = SA_NODEFER|SA_RESETHAND;
  
  int reg = sigaction(SIGSEGV, &sa, &osa);
  if (reg < 0) {
      LOG(ERROR) << "Unable to set sigaction in address Valid: " << errno << "\n";
      die();
    }

  char a __attribute__((unused)) = *static_cast<char *>(vptr);
  reg = sigaction(SIGSEGV, &osa, &sa);
  if (reg < 0) {
      LOG(ERROR) << "Unable to reset sigaction in addressValid: " << errno << "\n";
      die();
  }
  
  return b;
}

}} //namespace

