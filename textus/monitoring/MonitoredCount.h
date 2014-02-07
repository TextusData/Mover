/* MonitoredCount.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
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

