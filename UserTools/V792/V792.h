#ifndef V792_H
#define V792_H

#include <memory>

#include <caen++/v792.hpp>

#include "Tool.h"
#include "ThreadLoop.h"

class V792: public ToolFramework::Tool {
  public:
    ~V792();

    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    std::vector<caen::V792> qdcs;
    caen::V792::Buffer buffer;
    std::unique_ptr<ThreadLoop::handle> thread;

    void connect();
    void configure();
    void readout();
};

#endif
