#ifndef V1290_H
#define V1290_H

#include <memory>

#include <caen++/v1290.hpp>

#include "Tool.h"
#include "ThreadLoop.h"

class V1290: public ToolFramework::Tool {
  public:
    ~V1290();

    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    std::vector<caen::V1290> tdcs;
    caen::V1290::Buffer buffer;
    std::unique_ptr<ThreadLoop::handle> thread;

    void connect();
    void configure();
    void readout();
};

#endif
