#ifndef CAEN_V1742_H
#define CAEN_V1742_H

#include <array>
#include <string>
#include <vector>

#include <cstdarg>

#include <TFile.h>
#include <TTree.h>

#include "Tool.h"

#include "caen/digitizer.hpp"

class CAEN_V1742: public Tool {
  struct Args: public Thread_args {
    CAEN_V1742* tool = nullptr;

    caen::Digitizer*                digitizer = nullptr;
    caen::Digitizer::ReadoutBuffer* buffer    = nullptr;
    caen::Digitizer::WaveEvent*     event     = nullptr;

    TFile*   output = nullptr;
    TTree*   tree   = nullptr;
    TBranch* timestamp;
    std::vector<std::vector<double>> waveforms;
  };

  public:
    CAEN_V1742(): Tool(), util(nullptr), args(nullptr) {};

    bool Initialise(std::string configfile, DataModel& data);
    bool Execute();
    bool Finalise();

    // Log message at error level
    void error(const char*, ...);

    // Log message at warn level (default)
    void warn(const char*, ...);

    // Log message at info level (verbose)
    void info(const char*, ...);

    // Log message at custom level
    void log(int level, const char*, ...);
    void log(int level, const char*, va_list);

    void stop();

  private:
    Utilities* util;
    Args*      args;

    void configure(caen::Digitizer&);

    static void thread(Thread_args* arg);
};

#endif
