#include "Store.h"
#include "DataModel.h"

#include "V1290.h"

void V1290::connect() {
  caen::Device::Connection connection = caen_connection(m_variables);
  caen_report_connection(*m_log << ML(3), "V1290", connection);
  tdc.reset(new caen::V1290(connection));
  *m_log << ML(3) << "connected" << std::endl;
};

void V1290::configure() {
  *m_log << ML(3) << "Configuring V1290... " << std::flush;
  tdc->reset();
  sleep(1); // the board takes approximately 1 second to reset

  bool  flag;
  int   i;
  float x;
  std::string s;
  std::string var;
#define cfgvar(name, var) \
  if (m_variables.Get(#name, var)) tdc->set_ ## name(var)
#define cfgbool(name)  cfgvar(name, flag)
#define cfgint(name)   cfgvar(name, i)
#define cfgfloat(name) cfgvar(name, x)

  cfgbool(bus_error_enabled);
  cfgbool(sw_termination);
  cfgbool(sw_termination_enabled);
  cfgbool(emit_empty_events);
  cfgbool(align_64);
  cfgbool(compensation_enabled);
  cfgbool(ettt_enabled);
  cfgint(interrupt_level);
  cfgint(interrupt_vector);
  cfgint(geo_address);
  cfgbool(triggered_mode);
  cfgfloat(window_width);
  cfgfloat(window_offset);
  cfgfloat(search_margin);
  cfgfloat(reject_margin);
  cfgbool(trigger_time_subtraction);

  if (m_variables.Get("edge_detection", s))
    if (s == "leading")
      tdc->set_edge_detection(true, false);
    else if (s == "trailing")
      tdc->set_edge_detection(false, true);
    else if (s == "both")
      tdc->set_edge_detection(true, true);
    else
      throw std::runtime_error(
          std::string("V1290: invalid edge_detection setting: ") + s
      );

  {
    float edge  = 0;
    float pulse = 0;
    bool fedge =  m_variables.Get("edge_resolution", edge)
               || m_variables.Get("resolution", edge);
    bool fpulse = m_variables.Get("pulse_resolution", pulse);
    if (fedge || fpulse) {
      if (!fedge || !fpulse) {
        auto r = tdc->resolution();
        if (!fedge)  edge  = r.edge;
        if (!fpulse) pulse = r.pulse;
      };
      tdc->set_resolution(edge, pulse);
    };
  };

  cfgfloat(dead_time);
  cfgbool(header_and_trailer_enabled);
  cfgint(event_size);
  if (m_variables.Get("enable_error_mask",   flag)) tdc->enable_error_mark(flag);
  if (m_variables.Get("enable_error_bypass", flag)) tdc->enable_error_bypass(flag);

  {
    auto mask = tdc->internal_errors();
    auto old_mask = mask;
#define defbit(name) \
    if (m_variables.Get("enable_" #name "_error", flag)) mask.set_ ## name(flag)
    defbit(vernier);
    defbit(coarse);
    defbit(channel);
    defbit(l1_parity);
    defbit(trigger_fifo);
    defbit(readout_fifo);
    defbit(readout);
    defbit(setup);
    defbit(control);
    defbit(jtag);
#undef defbit
    if (mask != old_mask) tdc->set_internal_errors(mask);
  };

  cfgint(fifo_size);

  if (m_variables.Get("enabled_channels", s)) {
    size_t j;
    uint32_t mask = std::stol(s, &j, 16);
    if (j != s.size())
      throw std::runtime_error(
          std::string("V1290: invalid value for enabled_channels: ") + s
      );
    tdc->enable_channels(mask);
  };

  var = "enabled_channels_x";
  for (uint8_t j = 0; j < 4; ++j) {
    var.back() = '0' + j;
    uint32_t mask;
    if (m_variables.Get(var, mask)) tdc->enable_tdc_channels(j, mask);
  };

  {
    uint16_t coarse = 0;
    uint16_t fine   = 0;
    bool set = m_variables.Get("global_offset_coarse", coarse);
    set = m_variables.Get("global_offset_fine", fine) || set;
    if (set) tdc->set_global_offset(coarse, fine);
  };

  var = "adjust_channel_xx";
  for (uint8_t channel = 0; channel < 32; ++channel) {
    snprintf(&var.back() - 1, 2, "%hhu", channel);
    uint8_t adjust;
    if (m_variables.Get(var, adjust)) tdc->adjust_channel(channel, adjust);
  };

  var = "adjust_rc_x";
  for (uint8_t j = 0; j < 4; ++j) {
    var.back() = '0' + j;
    uint8_t set;
    if (m_variables.Get(var, set)) tdc->adjust_rc(j, set);
  };

  if (m_variables.Get("load_scan_path", flag))
    if (flag)
      tdc->scan_path_load();

  var = "load_scan_path_tdc_x";
  for (uint8_t j = 0; j < 4; ++j) {
    var.back() = '0' + j;
    if (m_variables.Get(var, flag))
      if (flag)
        tdc->scan_path_load(j);
  };

  if (m_variables.Get("dll_clock", s)) {
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
    tdc->set_dll_clock(clock);
  };
#undef cfgfloat
#undef cfgint
#undef cfgbool
#undef cfgvar

  *m_log << ML(3) << "success" << std::endl;
};

void V1290::readout() {
  tdc->readout(buffer);
  if (buffer.size() == 0) {
    usleep(100);
    return;
  };

  std::vector<caen::V1290::Packet> data(buffer.begin(), buffer.end());

  std::lock_guard<std::mutex> lock(m_data->v1290_mutex);
  m_data->v1290_readout.push_back(std::move(data));
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
  if (!tdc) return false;
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
    if (tdc) delete tdc.release();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};
