/* SysInfo.cc -*- c++ -*-
 * Copyright (c) 2010 Ross Biro
 *
 * Handles all the OS specific stuff of getting number of cpus, cpu packages, etc.
 */

#include "textus/system/SysInfo.h"

namespace textus { namespace system {

SysInfo *SysInfo::systemInformation() {
  static SysInfo si;
  return &si;
}

}} //namespace
