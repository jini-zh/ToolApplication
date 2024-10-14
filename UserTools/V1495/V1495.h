#ifndef V1495_H
#define V1495_H

#include <caen++/v1495.hpp>

#include "Tool.h"

class V1495: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise();

  private:
    caen::V1495* board = nullptr;
    std::vector<uint16_t> counters;

    void connect();
    void configure();
    void read_counters();
};

#endif
