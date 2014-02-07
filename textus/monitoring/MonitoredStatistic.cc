/* MonitoredStatistic.cc -*- c++ -*-
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
      VARIABLEDESCRIPTION(count, MonitoredStatistic::getCount, number, statistic),
      VARIABLEDESCRIPTION(sum, MonitoredStatistic::getSum, longnumber, statistic),
      VARIABLEDESCRIPTION(sum_squared, MonitoredStatistic::getSumSquared, longnumber, statistic),
      VARIABLEDESCRIPTION(min, MonitoredStatistic::getMin, number, statistic),
      VARIABLEDESCRIPTION(max, MonitoredStatistic::getMax, number, statistic),
      VARIABLEDESCRIPTIONEND
    };

   variableDescription *MonitoredCount::variableDescription()
  {
    return desc;
  }

} //namespace
