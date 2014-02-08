/* Monitor.h -*- c++ -*-
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

#ifndef TEXTUS_MONITORNING_MONITOR_H_
#define TEXTUS_MONITORNING_MONITOR_H_

namespace textus::monitoring {
  class MonitoredCount: MonitoredVariable {
  private:
    int count;
    Time resetTime;

  public:
    MonitoredCount():count(0), resetTime(Time::now()) {
      MonitorServer::registerMonitoredVariable(this);
    }

    virtual ~MonitoredCount()
    {
      MonitorServer::DeregisterMonitoredVariable(this);
    }

    operator ++ (void) {
      Synchronized(this);
      count++;
    }

    
  };
  
} // namespace

#define MONITORED_COUNT(foo)

#endif // TEXTUS_MONITORNING_MONITOR_H_

