#ifndef V812_H
#define V812_H

#include <memory>
#include <map>

#include <caen++/v812.hpp>

#include "Tool.h"

class V812: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    std::vector<caen::V812> cfds;

    // VME address -> V812
    std::map<uint16_t, caen::V812*> pcfds;

    void connect();
    void configure();
};

#endif
