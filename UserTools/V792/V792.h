#ifndef V792_H
#define V792_H

#include <memory>

#include <caen++/v792.hpp>

#include "Tool.h"
#include "Utilities.h"
#include "caen.h"

class V792: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    struct Thread: public ToolFramework::Thread_args {
      V792& tool;

      Thread(V792& tool): tool(tool) {};
    };

    std::unique_ptr<caen::V792> qdc;
    caen::V792::Buffer buffer;
    std::unique_ptr<Thread> thread;
    ToolFramework::Utilities util;

    void connect();
    void configure();
    void readout();

    static void readout_thread(ToolFramework::Thread_args* arg);

    void run_readout();
};

#endif
