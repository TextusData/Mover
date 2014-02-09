/* SysInfo.h -*- c++ -*-
 * Copyright (c) 2010-2014 Ross Biro
 *
 * Handles all the OS specific stuff of getting number of cpus, cpu packages, etc.
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
  bool getCPUFlag(std::string flag);
};


}} //namespace

#endif //TEXTUS_SYSTEM_SYS_INFO_H_
