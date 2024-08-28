#include "DataModel.h"

#include "V812.h"
#include "caen.h"

void V812::connect() {
  auto connections = caen_connections(m_variables);
  cfds.reserve(connections.size());
  for (auto& connection : connections) {
    caen_report_connection(*m_log << ML(3), "V812", connection);
    cfds.emplace_back(connection);
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

void V812::configure() {
  *m_log << ML(3) << "Configuring V812... " << std::flush;

  for (int cfd_index = 0; cfd_index < cfds.size(); ++cfd_index) {
    caen::V812& cfd = cfds[cfd_index];

    std::string s;
    bool flag;
    float x;
    int i;

    {
      uint32_t mask = 0;
      bool mask_set = false;
      if (cfg_get(m_variables, "enable_channels", cfd_index, s)) {
        mask_set = true;
        size_t end;
        mask = std::stol(s, &end, 16);
        if (end != s.size())
          throw std::runtime_error(
              std::string("V812: invalid value for enable_channels: ") + s
          );
      };

      std::stringstream ss;
      for (uint8_t channel = 0; channel < 16; ++channel) {
        ss.str({});
        ss << "enable_channel_" << static_cast<int>(channel);
        if (cfg_get(m_variables, ss.str(), cfd_index, flag)) {
          mask_set = true;
          uint16_t bit = 1 << channel;
          if (flag) mask |=  bit;
          else      mask &= ~bit;
        };

        ss.str({});
        ss << "channel_" << static_cast<int>(channel) << "_threshold";
        if (cfg_get(m_variables, ss.str(), cfd_index, x)) cfd.set_threshold(channel, x);
      };

      if (mask_set) cfd.enable_channels(mask);
    };

    if (cfg_get(m_variables, "output_width", cfd_index, i))
      cfd.set_output_width(i);
    if (cfg_get(m_variables, "output_width_0-7", cfd_index, i))
      cfd.set_output_width(0, i);
    if (cfg_get(m_variables, "output_width_8-15", cfd_index, i))
      cfd.set_output_width(1, i);

    if (cfg_get(m_variables, "dead_time", cfd_index, i))
      cfd.set_dead_time(i);
    if (cfg_get(m_variables, "dead_time_0-7", cfd_index, i))
      cfd.set_dead_time(0, i);
    if (cfg_get(m_variables, "dead_time_8-15", cfd_index, i))
      cfd.set_dead_time(1, i);

    if (cfg_get(m_variables, "majority_threshold", cfd_index, i))
      cfd.set_majority_threshold(i);
  };

  *m_log << ML(3) << "success" << std::endl;
};

bool V812::Initialise(std::string configfile, DataModel& data) {
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

bool V812::Finalise() {
  try {
    cfds.clear();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
}
