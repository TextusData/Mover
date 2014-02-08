/* MonitoredString.h -*- c++ -*-
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

#ifndef TEXTUS_MONITORNING_MONITORED_STRING_H_
#define TEXTUS_MONITORNING_MONITORED_STRING_H_

namespace textus::monitoring {
  class MonitoredString: MonitoredVariable {
  private:
    String value;

  public:
    MonitoredString(String name):MonitoredVariable(name), string(""), reset_time(Time::now()) {
      MonitorServer::registerMonitoredVariable(this);
    }

    virtual ~MonitoredString()
    {
      MonitorServer::DeregisterMonitoredVariable(this);
    }

    MonitoredString &operator =(const String &new_value) {
      value = new_value;
    }

    int getValue() {
      Synchronized(this);
      return value;
    }

    virtual void reset() { string = ""; super.reset(); }
    
  };
  
} // namespace


#endif // TEXTUS_MONITORNING_MONITORED_STRING_H_

