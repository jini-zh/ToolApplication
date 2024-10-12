#include "VMEControl.h"

#include "caen.h"
#include "DataModel.h"

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

void VMEControl::configure() {
  if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

  std::string string;
  if (m_variables.Get("link", string))
    connection.link = caen_connection_string_to_type(string);
  else
    connection.link = CAENComm_USB;

  m_variables.Get("ip",    connection.ip);
  m_variables.Get("conet", connection.conet);
  m_variables.Get("arg",   connection.arg);

  if (!m_variables.Get("vme", string))
    throw std::runtime_error(
        "V1495 (FPGA) VME address is not provided in the config"
    );
  fpga_address = str_to_uint16(string, 16);
};

static void write16(
    caen::Device& device,
    uint16_t address,
    const std::string& value
) {
  device.write16(address, str_to_uint16(value, 16));
};

static void write32(
    caen::Device& device,
    uint16_t address,
    const std::string& value
) {
  device.write32(address, str_to_uint32(value, 16));
};

caen::Device VMEControl::connect(const char* name, uint16_t vme) {
  connection.vme = vme << 16;
  caen_report_connection(*m_log << ML(3), name, connection);
  return caen::Device(connection);
};

void VMEControl::control(
    ToolFramework::Store& json,
    const char*           name,
    uint16_t              vme,
    bool                  wide
) {
  auto device = connect(name, vme);
  auto write = wide ? write32 : write16;
  for (auto& kv : json)
    write(device, str_to_uint16(kv.first, 16), json.Get<std::string>(kv.first));
};

std::string VMEControl::control(const char* field, bool fpga) {
  try {
    std::string json_string;
    m_data->sc_vars[field]->GetValue(json_string);

    ToolFramework::Store json;
    json.JsonParser(json_string);

    if (fpga)
      control(json, "V1495", fpga_address, true);
    else
      for (auto& kv : json)
        try {
          ToolFramework::Store settings;
          std::string s = json.Get<std::string>(kv.first);
          settings.JsonParser(json.Get<std::string>(kv.first));
          control(settings, "V812", str_to_uint16(kv.first, 16), false);
        } catch (std::exception& e) {
          std::stringstream ss;
          ss << kv.first << ": " << e.what();
          std::string msg = ss.str();
          *m_log << ML(0) << msg << std::endl;
          return msg;
        }

    return "ok";
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return e.what();
  };
};

std::string VMEControl::set_counters(const char* field) {
  try {
    std::string value;
    m_data->sc_vars[field]->GetValue(value);

    counters.clear();
    std::stringstream ss(value);
    while (true) {
      ss >> value;
      if (!ss) break;
      counters.push_back(str_to_uint16(value, 16));
    };
    return "ok";
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return e.what();
  };
};

/* Following does not work because ToolFramework::Store cannot decode a string
 * it had encoded. Here is what sendcommand2nopadding in WebServer sees for
 * return "ok {\"0x321c\":\"42\",\"0xdead\":\"54\"}":
 *
 * got response: '{"msg_type":"Command Reply" ,"msg_value":"ok {"0x321c":"42","0xdead":"54"}" }'
 * got repsonse '"ok{"0x321c":"42","0xdead":"54"}'
 */
#if 0
std::string VMEControl::get_counters() {
  try {
    ToolFramework::Store store;
    auto fpga = connect("V1495", fpga_address);
    std::string address(6, '\0');
    for (auto& counter : counters) {
      snprintf(&address[0], address.size() + 1, "0x%hx", counter);
      store.Set(address, fpga.read32(counter));
    };

    std::string json;
    store >> json;
    return "ok " + json;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return e.what();
  };
};
#endif

std::string VMEControl::get_counters() {
  try {
    std::stringstream result;
    result << "ok";
    auto fpga = connect("V1495", fpga_address);
    std::string address(6, '\0');
    for (auto& counter : counters) {
      snprintf(&address[0], address.size() + 1, "0x%hx", counter);
      result << ' ' << counter << ' ' << fpga.read32(counter);
    };

    return result.str();
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return e.what();
  };
};

bool VMEControl::Initialise(std::string configfile, DataModel& data) {
  InitialiseTool(data);
  InitialiseConfiguration(std::move(configfile));

  configure();

  m_data->sc_vars.Add(
      "V1495-get-counters",
      BUTTON,
      [this](const char*) -> std::string {
        return get_counters();
      }
  );

  m_data->sc_vars.Add(
      "V1495-set-counters",
      COMMAND,
      [this](const char* field) -> std::string {
        return set_counters(field);
      }
  );

  m_data->sc_vars.Add(
      "V1495-reset-counters",
      BUTTON,
      [this](const char*) -> std::string {
        try {
          connect("V1495", fpga_address).write32(0x3002, 1);
          return "ok";
         } catch (std::exception& e) {
          *m_log << ML(0) << e.what() << std::endl;
          return e.what();
         };
      }
  );

  m_data->sc_vars.Add(
      "V1495-set-registers",
      COMMAND,
      [this](const char* field) -> std::string {
        return control(field, true);
      }
  );

  m_data->sc_vars.Add(
      "V812-set-registers",
      COMMAND,
      [this](const char* field) -> std::string {
        return control(field, false);
      }
  );

  return true;
};

bool VMEControl::Execute() {
  if (m_data->change_config) {
    InitialiseConfiguration();
    configure();
  };

  return true;
};
