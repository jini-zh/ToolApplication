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
    std::vector<caen::V812> cfds;

    void connect();
    void configure();
};

#endif
