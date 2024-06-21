#include "DataModel.h"

#include "V812.h"
#include "caen.h"

void V812::connect() {
  auto connection = caen_connection(m_variables);
  caen_report_connection(*m_log << ML(3), "V821", connection);
  cfd.reset(new caen::V812(connection));
  *m_log << ML(3) << "connected" << std::endl;
};

void V812::configure() {
  *m_log << ML(3) << "Configuring V812... " << std::flush;

  std::string s;
  bool flag;
  float x;
  int i;
  
  uint16_t mask = ~0;
  bool mask_set = false;
  if (m_variables.Get("enable", s)) {
    size_t j;
    mask = std::stol(s, &j, 16);
    mask_set = true;
    if (j != s.size())
      throw std::runtime_error(
          std::string("V812: invalid value for enable_channels: ") + s
      );
  };

  std::stringstream ss;
  for (uint8_t channel = 0; channel < 16; ++channel) {
    ss.str({});
    ss << "enable_" << channel;
    if (m_variables.Get(ss.str(), flag)) {
      uint32_t bit = 1 << channel;
      if (flag) mask |= bit;
      else      mask &= ~bit;
      mask_set = true;
    };

    ss.str({});
    ss << "threshold_" << channel;
    if (m_variables.Get(ss.str(), x)) cfd->set_threshold(channel, x);
  };

  if (mask_set) cfd->enable_channels(mask);

  if (m_variables.Get("output_width_0-7", i))
    cfd->set_output_width(0, i);
  if (m_variables.Get("output_width_8-15", i))
    cfd->set_output_width(1, i);
  if (m_variables.Get("output_width", i))
    cfd->set_output_width(i);

  if (m_variables.Get("dead_time_0-7", i))
    cfd->set_dead_time(0, i);
  if (m_variables.Get("dead_time_8-15", i))
    cfd->set_dead_time(1, i);
  if (m_variables.Get("dead_time", i))
    cfd->set_dead_time(i);

  if (m_variables.Get("majority_threshold", i))
    cfd->set_majority_threshold(i);

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
    if (cfd) delete cfd.release();

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
}
