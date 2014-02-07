/* MonitorInternal.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Define Variables that are exported to the monitoring system.
 *
 */

#ifndef TEXTUS_MONITORING_MONITOR_INTERNAL_H_
#define TEXTUS_MONITORING_MONITOR_INTERNAL_H_

enum variableType {
  Invalid = -1,
  text=0,
  number,
  longnumber,
  longfloat,
};

enum recordType {
  Invalid = -1,
  storeall = 0, /* keep them all in a relational db. */
  statistic,    /* store in an rrdb keeping min, max, average and variance. */
  storesome,    /* keep them in an relational db, but delete some as space requires. */
};

struct variableDescription {
  String name;
  monitoredMemberPtr accessor;
  enum variableType variable_type;
  enum recordType record_type;
};

#define VARIABLEDESCRIPTIONEND {"", { &MonitoredVariable::invalidGet }, variableType::Invalid, recordType::Invalid }
#define VARIABLEDESCRIPTION(n, g, t, s) { #n, { &g }, t, s}


#endif // TEXTUS_MONITORING_MONITOR_INTERNAL_H_
