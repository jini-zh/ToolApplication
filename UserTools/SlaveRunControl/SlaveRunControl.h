#ifndef ALERT_SUB_H
#define ALERT_SUB_H

#include "Tool.h"
#include "Services.h"

class SlaveRunControl: public ToolFramework::Tool {
  public:
    SlaveRunControl(): context(1) {};

    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise() { return true; }

  private:
    zmq::context_t context;
    ToolFramework::SlowControlCollection scc;
    ToolFramework::Services services;
    bool run_start;
    bool run_stop;
    bool change_config;
    boost::posix_time::ptime start_time;
    unsigned int run_configuration;
};

#endif
