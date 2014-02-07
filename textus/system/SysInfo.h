/* SysInfo.h -*- c++ -*-
 * Copyright (c) 2010-2014 Ross Biro
 *
 * Handles all the OS specific stuff of getting number of cpus, cpu packages, etc.
 */

#ifndef TEXTUS_SYSTEM_SYS_INFO_H_
#define TEXTUS_SYSTEM_SYS_INFO_H_

#include <string>

namespace textus { namespace system {

// Should only ever be one instance of this class.
class SysInfo {
private:
  SysInfo() {};

protected:
public:
  static SysInfo *systemInformation();
  ~SysInfo() {}

  int getNumCPUs();
  std::string getAppPath();
  void random(unsigned char *buff, unsigned len);
  bool addressValid(void *vptr);
  std::string getUser();
};


}} //namespace

#endif //TEXTUS_SYSTEM_SYS_INFO_H_
