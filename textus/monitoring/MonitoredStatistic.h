/* MonitoredStatistic.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#ifndef TEXTUS_MONITORNING_MONITORED_STATISTIC_H_
#define TEXTUS_MONITORNING_MONITORED_STATISTIC_H_

namespace textus::monitoring {
  class MonitoredStatistic: public MonitoredVariable {
  private:
    int count;
    long long sum;
    long long sum_squared;
    int max;
    int min;
    
  public:
    MonitoredStatistic(String name): MonitoredVariable(name), count(0), sum(0), sum_squared(0), max(INT_MIN), min(INT_MAX);
    virtual ~MonitoredStatistic() {};
    
    MonitoredStatistic &operator = (int a) {
      Synchronized(this);
      count++;
      sum += a;
      sum_squared += a*a;
      if (a < min) {
	min = a;
      }

      if (a > max) {
	max = a;
      }

      return *this;
    }

    int getCount() {
      Synchronized(this);
      return count;
    }

    long long getSum() {
      Synchronized(this);
      return sum;
    }

    long long getSumSquared() {
      Synchronized(this);
      return sum_squared;
    }

    int getMin() {
      Synchronized(this);
      return min;
    }

    int getMax() {
      Synchronized(this);
      return max;
    }
  }

} //namespace
#endif //TEXTUS_MONITORNING_MONITORED_STATISTIC_H_
