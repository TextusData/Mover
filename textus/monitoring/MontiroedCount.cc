/* MonitoredCount.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#include "MonitoredCount.h"

namespace textus::monitoring {
  static struct variableDescription desc =
    {
      VARIABLEDESCRIPTION(count, getCount, number);
      VARIABLEDESCRIPTIONEND
    };

   variableDescription *MonitoredCount::variableDescription()
  {
    return desc;
  }

} //namespace
