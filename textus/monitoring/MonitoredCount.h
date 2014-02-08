/* MonitoredCount.h -*- c++ -*-
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

#ifndef TEXTUS_MONITORNING_MONITORED_COUNT_H_
#define TEXTUS_MONITORNING_MONITORED_COUNT_H_

namespace textus::monitoring {
  class MonitoredCount: MonitoredVariable {
  private:
    int count;

  public:
    MonitoredCount(String name):MonitoredVariable(name), count(0), reset_time(Time::now()) {
      MonitorServer::registerMonitoredVariable(this);
    }

    virtual ~MonitoredCount()
    {
      MonitorServer::DeregisterMonitoredVariable(this);
    }

    MonitoredCount &operator ++ (void) {
      Synchronized(this);
      count++;
      return *this;
    }

    MonitoredCount &operator -- (void) {
      Synchronized(this);
      count--;
      return *this;
    }

    int getCount() {
      Synchronized(this);
      return count;
    }

    virtual void reset() { count = 0; super.reset(); }
    
  };
  
} // namespace


#endif // TEXTUS_MONITORNING_MONITORED_COUNT_H_

