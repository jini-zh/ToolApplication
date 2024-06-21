#ifndef V1290_H
#define V1290_H

#include <memory>

#include <caen++/v1290.hpp>

#include "Tool.h"
#include "Utilities.h"

class V1290: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    struct Thread: public ToolFramework::Thread_args {
      V1290& tool;

      Thread(V1290& tool): tool(tool) {};
    };

    std::unique_ptr<caen::V1290> tdc;
    caen::V1290::Buffer buffer;
    std::unique_ptr<Thread> thread;
    ToolFramework::Utilities util;

    void connect();
    void configure();
    void readout();

    static void readout_thread(ToolFramework::Thread_args* arg);

    void run_readout();
};

#endif
