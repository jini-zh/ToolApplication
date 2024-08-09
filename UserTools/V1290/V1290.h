#ifndef V1290_H
#define V1290_H

#include <memory>

#include <caen++/v1290.hpp>

#include "Tool.h"
#include "ThreadLoop.h"

class V1290: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    std::vector<caen::V1290> tdcs;
    caen::V1290::Buffer buffer;
    ThreadLoop::Thread thread;

    void connect();
    void configure();
    void readout();
};

#endif
