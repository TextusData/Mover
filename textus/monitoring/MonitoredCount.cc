/* MonitoredCount.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#include "MonitoredCount.h"
#include "MonitorInternal.h"

namespace textus::monitoring {
  static struct variableDescription desc =
    {
      VARIABLEDESCRIPTION(count, MonitoredCount::getCount, number, statistic),
      VARIABLEDESCRIPTIONEND
    };

   variableDescription *MonitoredCount::variableDescription()
  {
    return desc;
  }

} //namespace
