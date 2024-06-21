#include "Store.h"
#include "DataModel.h"

#include "V792.h"
#include "caen.h"

void V792::connect() {
  caen::Device::Connection connection = caen_connection(m_variables);
  caen_report_connection(*m_log << ML(3), "V792", connection);
  qdc.reset(new caen::V792(connection));
  *m_log << ML(3) << "connected" << std::endl;
};

void V792::configure() {
  *m_log << ML(3) << "Configuring V792... " << std::flush;
  qdc->reset();
  qdc->clear();

  bool  flag;
  int   i;
  float x;
  std::string s;
  std::string var;
#define cfgvar(name, var) \
  if (m_variables.Get(#name, var)) qdc->set_ ## name(var)
#define cfgbool(name)  cfgvar(name, flag)
#define cfgint(name)   cfgvar(name, i)

  cfgint(geo_address);
  cfgint(interrupt_level);
  cfgint(interrupt_vector);

  {
    auto control = qdc->control1();
    auto old_control = control;
#define defbit(name) \
    if (m_variables.Get(#name, flag)) control.set_ ## name(flag)
    defbit(block_readout);
    defbit(panel_resets_software);
    defbit(bus_error_enabled);
    defbit(align_64);
#undef defbit
    if (old_control != control) qdc->set_control1(control);
  };

  cfgint(event_trigger);

  {
    auto bitset = qdc->bitset2();
    auto old_bitset = bitset;
#define defbit(name) \
    if (m_variables.Get(#name, flag)) bitset.set_ ## name(flag)
    defbit(overflow_enabled);
    defbit(threshold_enabled);
    defbit(slide_enabled);
    defbit(shift_threshold);
    defbit(empty_enabled);
    defbit(slide_subtraction_enabled);
    defbit(all_triggers);
#undef defbit
    if (old_bitset != bitset) qdc->set_bitset2(bitset);
  };

  cfgint(crate_number);
  cfgint(current_pedestal);
  cfgint(slide_constant);

  {
    uint32_t mask = 0;
    bool mask_set = false;
    if (m_variables.Get("enable_channels", s)) {
      size_t j;
      mask = std::stol(s, &j, 16);
      mask_set = true;
      if (j != s.size())
        throw std::runtime_error(
            std::string("V792: invalid value for enable_channels: ") + s
        );
    };

    std::stringstream ss;
    for (uint8_t channel = 0; channel < 32; ++channel) {
      ss.str({});
      ss << "channel_" << static_cast<int>(channel) << "_threshold";
      if (m_variables.Get(ss.str(), i))
        if (mask_set)
          qdc->set_channel_settings(channel, i, mask & 1 << channel);
        else
          qdc->set_channel_threshold(channel, i);
      else if (mask_set)
        qdc->set_channel_enabled(channel, mask & 1 << channel);
    };
  };
#undef cfgint
#undef cfgbool
#undef cfgvar

  *m_log << ML(3) << "success" << std::endl;
};

void V792::readout() {
  qdc->readout_wa(buffer);

  // Find the first Invalid packet, or skip to the end of the buffer
  uint32_t n = 0;
  while (n < buffer.size() && buffer[n].type() == caen::V792::Packet::Data)
    ++n;
  if (n < buffer.size() && buffer[n].type() == caen::V792::Packet::EndOfBlock)
    ++n;
  while (n < buffer.size() && buffer[n].type() == caen::V792::Packet::Header)
    n += buffer[n].as<caen::V792::Header>().count() + 2;

  if (n == 0) {
    usleep(100);
    return;
  };

  if (n > buffer.size()) n = buffer.size();

  std::vector<caen::V792::Packet> data(buffer.begin(), buffer.begin() + n);

  std::lock_guard<std::mutex> lock(m_data->v792_mutex);
  m_data->v792_readout.push_back(std::move(data));
};

void V792::readout_thread(Thread_args* arg) {
  Thread* thread = static_cast<Thread*>(arg);
  V792& tool = thread->tool;
  try {
    tool.readout();
  } catch (std::exception& e) {
    *tool.m_log << tool.ML(0) << e.what() << std::endl;
    thread->kill = true;
  };
};

void V792::run_readout() {
  thread.reset(new Thread(*this));
  util.CreateThread("V792", &readout_thread, thread.get());
};

bool V792::Initialise(std::string configfile, DataModel& data) {
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

bool V792::Execute() {
  if (!qdc) return false;
  try {
    run_readout();
    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

bool V792::Finalise() {
  try {
    if (thread) {
      util.KillThread(thread.get());
      delete thread.release();
    };
    if (qdc) delete qdc.release();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};
