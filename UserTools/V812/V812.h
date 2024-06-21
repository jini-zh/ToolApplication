#ifndef V812_H
#define V812_H

#include <memory>

#include <caen++/v812.hpp>

#include "Tool.h"

class V812: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute() { return true; };
    bool Finalise();

  private:
    std::unique_ptr<caen::V812> cfd;

    void connect();
    void configure();
};

#endif
