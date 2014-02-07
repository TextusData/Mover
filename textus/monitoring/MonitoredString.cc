/* MonitoredCount.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#include "MonitoredString.h"
#include "MonitorInternal.h"

namespace textus::monitoring {
  static struct variableDescription desc =
    {
      VARIABLEDESCRIPTION(value, MonitoredString::getValue, string, storeall),
      VARIABLEDESCRIPTIONEND
    };

   variableDescription *MonitoredString::variableDescription()
  {
    return desc;
  }

} //namespace
