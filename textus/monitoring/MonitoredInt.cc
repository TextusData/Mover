/* MonitoredCount.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#include "MonitoredInt.h"
#include "MonitorInternal.h"

namespace textus::monitoring {
  static struct variableDescription desc =
    {
      VARIABLEDESCRIPTION(value, MonitoredInt::getValue, int, storeall),
      VARIABLEDESCRIPTIONEND
    };

   variableDescription *MonitoredInt::variableDescription()
  {
    return desc;
  }

} //namespace
