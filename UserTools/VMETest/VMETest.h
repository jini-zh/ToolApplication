#ifndef TEST_TOOL_H
#define TEST_TOOL_H

#include <thread>

#include "Tool.h"

class VMETest: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    std::thread thread;
    bool stop = true;

    void start_generation();
    void stop_generation();
};

#endif
