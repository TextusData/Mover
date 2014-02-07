/* Monitor.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
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

