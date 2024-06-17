#include "caen.h"

CAENComm_ConnectionType caen_connection_string_to_type(const std::string& string) {
  if (string == "usb")
    return CAENComm_USB;
  if (string == "optical")
    return CAENComm_OpticalLink;
  if (string == "A4818-V2718")
    return CAENComm_USB_A4818_V2718;
  if (string == "A4818-V3718")
    return CAENComm_USB_A4818_V3718;
  if (string == "A4818-V4718")
    return CAENComm_USB_A4818_V4718;
  if (string == "eth-V4718")
    return CAENComm_ETH_V4718;
  if (string == "usb-V4718")
    return CAENComm_USB_V4718;
  return static_cast<CAENComm_ConnectionType>(-1);
};

const char* caen_connection_type_to_string(CAENComm_ConnectionType type) {
  switch (type) {
    case CAENComm_USB:
      return "usb";
    case CAENComm_OpticalLink:
      return "optical";
    case CAENComm_USB_A4818_V2718:
      return "A4818-V2718";
    case CAENComm_USB_A4818_V3718:
      return "A4818-V3718";
    case CAENComm_USB_A4818_V4718:
      return "A4818-V4718";
    case CAENComm_ETH_V4718:
      return "eth-V4718";
    case CAENComm_USB_V4718:
      return "usb-V4718";
    default:
      return "unknown";
  };
};

caen::Device::Connection caen_connection(
    ToolFramework::Store& variables, const std::string& prefix
) {
  std::stringstream ss;
  std::string string;
  ss << prefix << "link";
  if (!variables.Get(ss.str(), string))
    throw std::runtime_error(
        std::string("Cannot connect to ")
        + prefix
        + ": config variable `"
        + ss.str()
        + "' not found"
    );

  caen::Device::Connection c;
  c.type = caen_connection_string_to_type(string);
  if (c.type < 0) {
    ss << ": invalid connection type: " << string;
    throw std::runtime_error(ss.str());
  };

  ss.str(prefix);
  if (c.is_ethernet()) {
    ss << "ip";
    if (!variables.Get(ss.str(), c.ip)) {
      ss << " is not found in the configuration file";
      throw std::runtime_error(ss.str());
    };

    return c;
  };

  ss.str(prefix);
  ss << "arg";
  variables.Get(ss.str(), c.arg);

  ss.str(prefix);
  ss << "conet";
  variables.Get(ss.str(), c.conet);

  ss.str(prefix);
  ss << "vme";
  variables.Get(ss.str(), string);
  c.vme = std::stoi(string, nullptr, 16);

  return c;
};

void caen_report_connection(
    ToolFramework::Logging& log,
    const std::string& device,
    const caen::Device::Connection& connection
) {
  log
    << "Connecting to "
    << device
    << " (link = " << caen_connection_type_to_string(connection.type);
  if (connection.is_ethernet())
    log << ", ip = " << connection.ip;
  else
    log
      << ", arg = "
      << connection.arg
      << ", conet = "
      << connection.conet
      << ", vme = 0x"
      << std::hex
      << connection.vme
      << std::dec;
  log << ')' << std::endl;
};
