#ifndef VMECONTROL_H
#define VMECONTROL_H

#include <caen++/caen.hpp>

#include "Tool.h"

class VMEControl: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel&);
    bool Execute();
    bool Finalise() { return true; };

  private:
    caen::Device::Connection connection;
    uint16_t fpga_address;
    std::vector<uint16_t> counters;

    void configure();

    caen::Device connect(const char* name, uint16_t vme);

    void control(
        ToolFramework::Store& json,
        const char*           name,
        uint16_t              vme,
        bool                  wide
    );

    std::string control(const char* field, bool fpga);
    std::string set_counters(const char* field);
    std::string get_counters();
};

#endif
