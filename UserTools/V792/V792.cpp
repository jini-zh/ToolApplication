#include "Store.h"
#include "DataModel.h"

#include "V792.h"
#include "caen.h"

void V792::connect() {
  auto connections = caen_connections(m_variables);
  qdcs.reserve(connections.size());
  for (auto& connection : connections) {
    caen_report_connection(*m_log << ML(3), "V792", connection);
    qdcs.emplace_back(connection);
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

void V792::configure() {
  *m_log << ML(3) << "Configuring V792... " << std::flush;
  for (int qdc_index = 0; qdc_index < qdcs.size(); ++qdc_index) {
    caen::V792& qdc = qdcs[qdc_index];
    qdc.reset();
    qdc.clear();

    bool  flag;
    int   i;
    float x;
    std::string s;
    std::string var;
#define cfgvar(name, var) \
    if (cfg_get(m_variables, #name, qdc_index, var)) qdc.set_ ## name(var)
#define cfgbool(name)  cfgvar(name, flag)
#define cfgint(name)   cfgvar(name, i)

    cfgint(geo_address);
    cfgint(interrupt_level);
    cfgint(interrupt_vector);

    {
      auto control = qdc.control1();
      auto old_control = control;
#define defbit(name) \
      if (m_variables.Get(#name, flag)) control.set_ ## name(flag)
      defbit(block_readout);
      defbit(panel_resets_software);
      if (m_variables.Get("enable_bus_error", flag))
        control.set_bus_error_enabled(flag);
      defbit(align_64);
#undef defbit
      if (old_control != control) qdc.set_control1(control);
    };

    cfgint(event_trigger);

    {
      auto bitset = qdc.bitset2();
      auto old_bitset = bitset;
#define defbit(name) \
      if (m_variables.Get("enable_" #name, flag)) \
        bitset.set_ ## name ## _enabled(flag)
      defbit(overflow);
      defbit(threshold);
      defbit(slide);
      if (m_variables.Get("shift_threshold", flag))
        bitset.set_shift_threshold(flag);
      defbit(empty);
      defbit(slide_subtraction);
      if (m_variables.Get("all_triggers", flag))
        bitset.set_all_triggers(flag);
#undef defbit
      if (old_bitset != bitset) qdc.set_bitset2(bitset);
    };

    cfgint(crate_number);
    cfgint(current_pedestal);
    cfgint(slide_constant);

    {
      uint32_t mask = 0;
      bool mask_set = false;
      if (cfg_get(m_variables, "enable_channels", qdc_index, s)) {
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
        if (cfg_get(m_variables, ss.str(), qdc_index, i))
          if (mask_set)
            qdc.set_channel_settings(channel, i, mask & 1 << channel);
          else
            qdc.set_channel_threshold(channel, i);
        else if (mask_set)
          qdc.set_channel_enabled(channel, mask & 1 << channel);
      };
    };
#undef cfgint
#undef cfgbool
#undef cfgvar
  };

  *m_log << ML(3) << "success" << std::endl;
};

void V792::readout() {
  for (auto& qdc : qdcs) {
    qdc.readout_wa(buffer);

    // Find the first Invalid packet, or skip to the end of the buffer
    // TODO: move it to the processing tool
    uint32_t n = buffer.size();
    if (n > 0 && buffer[n-1].type() == caen::V792::Packet::Invalid)
      if (buffer[0].type() == caen::V792::Packet::Invalid)
        n = 0;
      else {
        // binary search
        uint32_t m = 0;
        while (n - m > 1) {
          uint32_t k = (n + m) / 2;
          if (buffer[k].type() == caen::V792::Packet::Invalid)
            n = k;
          else
            m = k;
        };
      };

    if (n == 0) continue;

    std::vector<caen::V792::Packet> data(buffer.begin(), buffer.begin() + n);

    std::lock_guard<std::mutex> lock(m_data->v792_mutex);
    m_data->v792_readout.push_back(std::move(data));
  };
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
  if (qdcs.empty()) return true;
  try {
    thread.reset(
        new ThreadLoop::handle(
          m_data->vme_readout.subscribe(
            [this]() -> bool {
              try {
                readout();
                return true;
              } catch (std::exception& e) {
                *m_log << ML(0) << e.what() << std::endl;
                return false;
              }
            }
          )
        )
    );
    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

bool V792::Finalise() {
  try {
    if (thread) {
      m_data->vme_readout.unsubscribe(*thread);
      delete thread.release();
    };

    qdcs.clear();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

V792::~V792() {
  if (thread) m_data->vme_readout.unsubscribe(*thread);
};
