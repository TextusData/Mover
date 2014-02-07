/* Monitor.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#ifndef TEXTUS_MONITORING_MONITORED_VARIABLED_H_
#define TEXTUS_MONITORING_MONITORED_VARIABLED_H_

namespace textus::monitoring {

  class MonitoredVariable: virtual public Base {
  private:
    String name;
    Time reset_time;
  public:
    typedef string (MonitoredVariable::*getStringValue)(void) const;
    typedef long (MonitoredVariable::*getLongValue)(void) const;
    typedef double(MonitoredVariable::*getDoubleValue)(void) const;
    typedef int (MonitoredVariable::*getIntValue)(void) const;

    typedef union {
      MonitoredVariable::getStringValue s;
      MonitoredVariable::getLongValue l;
      MonitoredVariable::getDoubleValue d;
      MonitoredVariable::getIntValue i;
    } monitoredMemberPtr;
		 

    MonitoredVariable(String n) :name(n), reset_time(Time::now()) {}
    virtual ~MontiroedVariable() {}

    virtual variableDescription *variableDescriptions()=0;

    const String &getName() const { return name; }
    const Time &getResetTime() const { return reset_time; }

    virtual void reset() { rest_time = Time::now(); }

    int invalidGet(void) const { 
      LOG(ERROR) << "MonitoredVariable::invalidGet called\n";
      LOG_CALL_STACK(ERROR);
      die();
    }
    
  };
} // namespace

#endif //TEXTUS_MONITORING_MONITORED_VARIABLED_H_
