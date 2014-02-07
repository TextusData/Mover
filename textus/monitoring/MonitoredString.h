/* MonitoredString.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
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

