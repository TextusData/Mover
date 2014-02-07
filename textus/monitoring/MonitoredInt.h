/* MonitoredInt.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 * This class is for integers that are to be stored indefinitely.
 * For integers where statistics make more sense, use
 * MonitoredStatistic
 */

#ifndef TEXTUS_MONITORNING_MONITORED_INT_H_
#define TEXTUS_MONITORNING_MONITORED_INT_H_

namespace textus::monitoring {
  class MonitoredInt: MonitoredVariable {
  private:
    int value;

  public:
    MonitoredInt(String name):MonitoredVariable(name), count(0) {
      MonitorServer::registerMonitoredVariable(this);
    }

    virtual ~MonitoredInt()
    {
      MonitorServer::DeregisterMonitoredVariable(this);
    }

    MonitoredInt &operator ++ (void) {
      Synchronized(this);
      value++;
      return *this;
    }

    MonitoredInt &operator = (int newValue) {
      Synchronized(this);
      value = newValue;
      return *this;
    }

    int getValue() {
      Synchronized(this);
      return Value;
    }

    virtual void reset() { value = 0; super.reset(); }
    
  };
  
} // namespace


#endif // TEXTUS_MONITORNING_MONITORED_INT_H_

