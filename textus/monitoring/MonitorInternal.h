/* MonitorInternal.h -*- c++ -*-
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
