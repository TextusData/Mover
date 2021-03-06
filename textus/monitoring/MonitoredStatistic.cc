/* MonitoredStatistic.cc -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
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
