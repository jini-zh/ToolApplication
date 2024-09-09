// A few utilities helpful when working with CAEN boards via caen++

#ifndef CAEN_H
#define CAEN_H

#include <string>
#include <list>

#include <caen++/caen.hpp>

#include "Store.h"
#include "Logging.h"

// Parses string and returns CAENComm_ConnectionType or -1 for invalid strings
CAENComm_ConnectionType
caen_connection_string_to_type(const std::string&);

// Provides a readable string describing the connection type
// (reverse of the above)
const char*
caen_connection_type_to_string(CAENComm_ConnectionType);

std::list<caen::Device::Connection> caen_connections(
    ToolFramework::Store& variables
);

// Prints a message to log describing connection information. Useful for
// verbosity logging of connection attempts.
void
caen_report_connection(
    ToolFramework::Logging& log,
    const std::string& device,
    const caen::Device::Connection& connection
);

#endif
