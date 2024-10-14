#include <fstream>
#include <limits>
#include <string>
#include <stdexcept>

#include <cstdlib>

#include "caen.h"
#include "DataModel.h"

#include "V1495.h"

static unsigned long str_to_ulong(const std::string& string, int base = 10) {
  char* end;
  unsigned long result = std::strtoul(string.c_str(), &end, base);
  if (*end)
    throw std::runtime_error(std::string("bad integer: ") + string);
  return result;
};

static uint16_t str_to_uint16(const std::string& string, int base = 10) {
  unsigned long result = str_to_ulong(string, base);
  if (result > std::numeric_limits<uint16_t>().max())
    throw std::runtime_error(std::string("uint16_t overflow: ") + string);
  return result;
};

static uint32_t str_to_uint32(const std::string& string, int base = 10) {
  unsigned long result = str_to_ulong(string, base);
  if (result > std::numeric_limits<uint32_t>().max())
    throw std::runtime_error(std::string("uint32_t overflow: ") + string);
  return result;
};

void V1495::connect() {
  caen::Device::Connection connection;

  std::string string;
  if (m_variables.Get("link", string))
    connection.link = caen_connection_string_to_type(string);
  else
    connection.link = CAENComm_USB;

  m_variables.Get("ip",    connection.ip);
  m_variables.Get("conet", connection.conet);
  m_variables.Get("arg",   connection.arg);
  if (m_variables.Get("vme", string)) {
    connection.vme = str_to_uint16(string, 16);
    connection.vme <<= 16;
  };

  caen_report_connection(*m_log << ML(3), "V1495", connection);
  board = new caen::V1495(connection);
};

void V1495::configure() {
  std::string registers;
  if (m_variables.Get("registers", registers)) {
    ToolFramework::Store json;
    json.JsonParser(registers);
    for (auto& kv : json)
      board->write32(
          str_to_uint16(kv.first, 16),
          str_to_uint32(json.Get<std::string>(kv.first), 16)
      );
  };

  std::vector<std::string> addresses;
  if (m_variables.Get("counters", addresses)) {
    counters.resize(addresses.size());
    for (size_t i = 0; i < addresses.size(); ++i)
      counters[i] = str_to_uint16(addresses[i], 16);
  } else
    counters.clear();
};

void V1495::read_counters() {
  Store store;
  std::string address(6, '\0');
  for (auto& counter : counters) {
    snprintf(&address[0], address.size() + 1, "0x%hx", counter);
    store.Set(address, board->read32(counter));
  };

  std::string json;
  store >> json;
  m_data->services->SendMonitoringData(json, "V1495");

  // Reset counters
  board->write32(0x3002, 1);
};

bool V1495::Initialise(std::string configfile, DataModel& data) {
  try {
    InitialiseTool(data);
    InitialiseConfiguration(std::move(configfile));

    if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

    connect();
    configure();

    if (m_data->services)
      m_data->services->AlertSubscribe(
          "CounterRead",
          [this](const char*, const char*) { read_counters(); }
      );

    ExportConfiguration();

    return true;
  } catch (std::exception& e) {
    if (m_log)
      *m_log << ML(0) << e.what() << std::endl;
    else
      fprintf(stderr, "%s\n", e.what());
    return false;
  };
};

bool V1495::Execute() {
  try {
    if (m_data->change_config) {
      InitialiseConfiguration();
      configure();
    };
    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

bool V1495::Finalise() {
  try {
    if (board) {
      delete board;
      board = nullptr;
    };

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};
