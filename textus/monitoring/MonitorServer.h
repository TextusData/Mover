/* MonitorServer.h -*- c++ -*-
 * Copyright (c) 2009 Ross Biro
 *
 * Exported to the monitoring system.
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
