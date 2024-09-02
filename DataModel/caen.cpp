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

std::list<caen::Device::Connection> caen_connections(
    ToolFramework::Store& variables
) {
  std::stringstream ss;
  std::string string;

  auto parse_link = [&](const std::string& string) -> CAENComm_ConnectionType {
    CAENComm_ConnectionType result = caen_connection_string_to_type(string);
    if (result >= 0) return result;

    ss.str({});
    ss << "caen_connections: invalid connection link: " << string;
    throw std::runtime_error(ss.str());
  };

  auto get_hex = [&](const std::string& name, uint32_t& value) -> bool {
    if (!variables.Get(name, string)) return false;
    size_t end;
    value = std::stoul(string, &end, 16);
    if (end == string.size()) return value;

    ss.str({});
    ss
      << "caen_connections: invalid hexadecimal value `"
      << string
      << "' for option "
      << name;
    throw std::runtime_error(ss.str());
  };

  caen::Device::Connection default_ { .link = CAENComm_USB };
  bool default_set = variables.Get("link", string);
  if (default_set) default_.link = parse_link(string);

  default_set = variables.Get("ip",    default_.ip)    || default_set;
  default_set = variables.Get("arg",   default_.arg)   || default_set;
  default_set = variables.Get("conet", default_.conet) || default_set;
  if (get_hex("vme", default_.vme)) {
    default_.vme <<= 16;
    default_set = true;
  };

  std::list<caen::Device::Connection> result;
  for (int i = 0; ; ++i) {
    caen::Device::Connection connection = default_;

    ss.str({});
    ss << "link_" << i;
    bool set = variables.Get(ss.str(), string);
    if (set) connection.link = parse_link(string);

    ss.str({});
    ss << "ip_" << i;
    set = variables.Get(ss.str(), connection.ip)    || set;

    ss.str({});
    ss << "arg_" << i;
    set = variables.Get(ss.str(), connection.arg)   || set;

    ss.str({});
    ss << "conet_" << i;
    set = variables.Get(ss.str(), connection.conet) || set;

    ss.str({});
    ss << "vme_" << i;
    set = get_hex(ss.str(), connection.vme)         || set;

    if (!set) break;

    result.push_back(connection);
  };

  if (result.empty() && default_set) result.push_back(default_);
  return result;
};

void caen_report_connection(
    ToolFramework::Logging& log,
    const std::string& device,
    const caen::Device::Connection& connection
) {
  log
    << "Connecting to "
    << device
    << " (link = " << caen_connection_type_to_string(connection.link);
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
