#include "Store.h"
#include "DataModel.h"

#include "V1290.h"
#include "caen.h"

void V1290::connect() {
  auto connections = caen_connections(m_variables);
  tdcs.reserve(connections.size());
  for (auto& connection : connections) {
    caen_report_connection(*m_log << ML(3), "V1290", connection);
    tdcs.emplace_back(connection);
  };
};

template <typename T>
static bool cfg_get(
    ToolFramework::Store& variables,
    std::string name,
    int index,
    T& var
) {
  std::stringstream ss;
  ss << name << '_' << index;
  return variables.Get(ss.str(), var) || variables.Get(std::move(name), var);
};

template <typename T>
static bool cfg_get(
    ToolFramework::Store& variables,
    std::string name,
    int index1,
    int index2,
    T& var
) {
  return cfg_get(variables, name + '_' + std::to_string(index1), index2, var);
};

static uint32_t cfg_hex_to_uint(
    const std::string& field, const std::string& string
) {
  size_t end;
  uint32_t hex = std::stol(string, &end, 16);
  if (end != string.size())
    throw std::runtime_error(
        std::string("V1290: invalid hexadecimal number for ")
        + field
        + ": "
        + string
    );
  return hex;
};

static bool cfg_get_hex(
    ToolFramework::Store& variables,
    std::string name,
    int index,
    uint32_t& var
) {
  std::string string;
  if (!cfg_get(variables, name, index, string)) return false;
  size_t end;
  var = cfg_hex_to_uint(name, string);
  return true;
};

static bool cfg_get_mask(
    ToolFramework::Store& variables,
    std::string name,
    int index,
    uint32_t& mask
) {
  std::string string;
  mask = 0;
  bool set = cfg_get_hex(variables, name + 's', index, mask);
  for (int i = 0; i < 32; ++i) {
    bool flag;
    if (cfg_get(variables, name, i, index, flag)) {
      set = true;
      uint32_t bit = 1 << i;
      if (flag) mask |=  bit;
      else      mask &= ~bit;
    };
  };
  fprintf(stderr, "cfg_get_mask %s => %x, %d\n", name.c_str(), mask, set);
  return set;
};

template <typename uint>
static inline void apply_adjusts(
    ToolFramework::Store& variables,
    caen::V1290& tdc,
    const char* name,
    int tdc_index,
    uint8_t number,
    void (caen::V1290::*adjust)(uint8_t, uint)
) {
  std::vector<uint> adjusts;
  std::string option("adjust_");
  option += name;
  cfg_get(variables, option + 's', tdc_index, adjusts);
  for (uint8_t i = 0; i < number; ++i) {
    bool set = i < adjusts.size();
    uint8_t value = set ? adjusts[i] : 0;
    set = cfg_get(variables, option, i, tdc_index, value) || set;
    if (set) (tdc.*adjust)(i, value);
  };
};

void V1290::configure() {
  *m_log << ML(3) << "Configuring V1290... " << std::flush;
  for (int tdc_index = 0; tdc_index < tdcs.size(); ++tdc_index) {
    caen::V1290& tdc = tdcs[tdc_index];
    tdc.reset();
    sleep(1); // the board takes approximately 1 second to reset

    bool     flag;
    int      i;
    float    x;
    uint32_t mask;
    std::string s;
    std::string var;
#define cfgvar(name, var) \
    if (cfg_get(m_variables, #name, tdc_index, var)) tdc.set_ ## name(var)
#define cfgbool(name)  cfgvar(name, flag)
#define cfgenable(name) \
    if (cfg_get(m_variables, "enable_" #name, tdc_index, flag)) \
      tdc.set_ ## name ## _enabled(flag)
#define cfgint(name)   cfgvar(name, i)
#define cfgfloat(name) cfgvar(name, x)

    cfgbool(triggered_mode);

    cfgenable(bus_error);
    cfgbool(sw_termination);
    cfgenable(sw_termination);
    cfgbool(emit_empty_events);
    cfgbool(align_64);
    cfgenable(compensation);
    cfgenable(ettt);
    cfgint(interrupt_level);
    cfgint(interrupt_vector);
    cfgint(geo_address);
    cfgfloat(window_width);
    cfgfloat(window_offset);
    cfgfloat(search_margin);
    cfgfloat(reject_margin);
    cfgbool(trigger_time_subtraction);

    if (cfg_get(m_variables, "edge_detection", tdc_index, s))
      if (s == "leading")
        tdc.set_edge_detection(true, false);
      else if (s == "trailing")
        tdc.set_edge_detection(false, true);
      else if (s == "both")
        tdc.set_edge_detection(true, true);
      else
        throw std::runtime_error(
            std::string("V1290: invalid edge_detection setting: ") + s
        );

    {
      float edge  = 0;
      float pulse = 0;
      bool fedge =  cfg_get(m_variables, "edge_resolution",  tdc_index, edge)
                 || cfg_get(m_variables, "resolution",       tdc_index, edge);
      bool fpulse = cfg_get(m_variables, "pulse_resolution", tdc_index, pulse);
      if (fedge || fpulse) {
        if (!fedge || !fpulse) {
          auto r = tdc.resolution();
          if (!fedge)  edge  = r.edge;
          if (!fpulse) pulse = r.pulse;
        };
        tdc.set_resolution(edge, pulse);
      };
    };

    cfgfloat(dead_time);
    cfgenable(header_and_trailer);
    cfgint(event_size);
    if (cfg_get(m_variables, "enable_error_mark", tdc_index, flag))
      tdc.enable_error_mark(flag);
    if (cfg_get(m_variables, "enable_error_bypass", tdc_index, flag))
      tdc.enable_error_bypass(flag);

    {
      auto mask = tdc.internal_errors();
      auto old_mask = mask;
#define defbit(name) \
      if (cfg_get(m_variables, "enable_" #name "_error", tdc_index, flag)) \
        mask.set_ ## name(flag)
      defbit(vernier);
      defbit(coarse);
      defbit(channel);
      defbit(l1_parity);
      defbit(trigger_fifo);
      defbit(trigger);
      defbit(readout_fifo);
      defbit(readout);
      defbit(setup);
      defbit(control);
      defbit(jtag);
#undef defbit
      if (mask != old_mask) tdc.set_internal_errors(mask);
    };

    cfgint(fifo_size);

    if (cfg_get_mask(m_variables, "enable_channel", tdc_index, mask))
      tdc.enable_channels(mask);

    {
      std::vector<std::string> tdc_channels;
      cfg_get(m_variables, "enable_tdc_channels", tdc_index, tdc_channels);

      std::stringstream ss;
      for (uint8_t j = 0; j < 4; ++j) {
        ss.str({});
        ss << "enable_tdc_" << static_cast<int>(j) << "_channel";
        s = ss.str();
        bool set = cfg_get_mask(m_variables, s, tdc_index, mask);
        if (!set && j < tdc_channels.size()) {
          mask = cfg_hex_to_uint(s, tdc_channels[j]);
          set = true;
        };
        if (set) tdc.enable_tdc_channels(j, mask);
      };
    };

    {
      uint16_t coarse = 0;
      uint16_t fine   = 0;
      bool set = cfg_get(
          m_variables, "global_offset_coarse", tdc_index, coarse
      );
      set = cfg_get(m_variables, "global_offset_fine", tdc_index, fine) || set;
      if (set) tdc.set_global_offset(coarse, fine);
    };

    apply_adjusts(
        m_variables, tdc, "channel", tdc_index, 32, &caen::V1290::adjust_channel
    );
    apply_adjusts(
        m_variables, tdc, "rc", tdc_index, 4, &caen::V1290::adjust_rc
    );

    if (cfg_get(m_variables, "load_scan_path", tdc_index, flag))
      if (flag)
        tdc.scan_path_load();

    for (uint8_t j = 0; j < 4; ++j)
      if (cfg_get(m_variables, "load_scan_path_tdc", j, tdc_index, flag))
        if (flag) tdc.scan_path_load(j);

    if (cfg_get(m_variables, "dll_clock", tdc_index, s)) {
      uint8_t clock;
      if (s == "direct_40")
        clock = 0;
      else if (s == "PLL_40")
        clock = 1;
      else if (s == "PLL_160")
        clock = 2;
      else if (s == "PLL_320")
        clock = 3;
      else
        throw std::runtime_error(
            std::string("V1290: invalid value for dll_clock: ") + s
        );
      tdc.set_dll_clock(clock);
    };
#undef cfgfloat
#undef cfgint
#undef cfgenable
#undef cfgbool
#undef cfgvar
  };

  *m_log << ML(3) << "success" << std::endl;
};

void V1290::readout() {
  for (auto& tdc : tdcs) {
    tdc.readout(buffer);
    if (buffer.empty()) continue;

    std::vector<caen::V1290::Packet> data(buffer.begin(), buffer.end());

    std::lock_guard<std::mutex> lock(m_data->v1290_mutex);
    m_data->v1290_readout.push_back(std::move(data));
  };
};

void V1290::readout_thread(Thread_args* arg) {
  Thread* thread = static_cast<Thread*>(arg);
  V1290& tool = thread->tool;
  try {
    tool.readout();
  } catch (std::exception& e) {
    *tool.m_log << tool.ML(0) << e.what() << std::endl;
    thread->kill = true;
  };
};

void V1290::run_readout() {
  thread.reset(new Thread(*this));
  util.CreateThread("V1290", &readout_thread, thread.get());
};

bool V1290::Initialise(std::string configfile, DataModel& data) {
  try {
    if (configfile != "") m_variables.Initialise(configfile);

    m_data = &data;
    m_log  = m_data->Log;

    if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

    connect();
    configure();

    return true;

  } catch (std::exception& e) {
    if (m_log)
      *m_log << ML(0) << e.what() << std::endl;
    else
      fprintf(stderr, "%s\n", e.what());
    return false;
  };
};

bool V1290::Execute() {
  if (tdcs.empty()) return true;
  try {
    run_readout();
    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

bool V1290::Finalise() {
  try {
    if (thread) {
      util.KillThread(thread.get());
      delete thread.release();
    };
    tdcs.clear();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};
