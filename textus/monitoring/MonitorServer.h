/* MonitorServer.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Exported to the monitoring system.
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

#ifndef TEXTUS_MONITORING_MONITORING_SERVER_H_
#define TEXTUS_MONITORING_MONITORING_SERVER_H_

namespace textus::monitoring
{
  class MonitoringServer: public SimpleServer {
  private:
  public:
    MonitoringServer() {}
    virtual ~MonitoringServer() {}

  }
};

#endif //TEXTUS_MONITORING_MONITORING_SERVER_H_
