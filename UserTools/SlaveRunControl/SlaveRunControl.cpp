#include <boost/date_time/posix_time/posix_time.hpp>

#include "SlaveRunControl.h"
#include "DataModel.h"

bool SlaveRunControl::Initialise(std::string configfile, DataModel& data) {
  try {
    InitialiseTool(data);
    InitialiseConfiguration(std::move(configfile));

    if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

    run_start     = m_data->run_start;
    run_stop      = m_data->run_stop;
    change_config = m_data->change_config;

    services.Init(m_variables, &context, &scc);

    services.AlertSubscribe(
        "RunStart",
        [this](const char* alert, const char* payload) {
          ToolFramework::Store store;
          store.JsonParser(payload);
          start_time = boost::posix_time::time_from_string(
              "1970-01-01 00:00:00.000"
          ) + boost::posix_time::seconds(store.Get<unsigned long>("Timestamp"));
          run_start = true;
        }
    );

    services.AlertSubscribe(
        "RunStop",
        [this](const char* alert, const char* payload) {
          run_stop = true;
        }
    );

    services.AlertSubscribe(
        "ChangeConfig",
        [this](const char* alert, const char* payload) {
          ToolFramework::Store store;
          store.JsonParser(payload);
          run_configuration = store.Get<unsigned int>("RunConfig");
          change_config = true;
        }
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

bool SlaveRunControl::Execute() {
  if (run_start)
    if (m_data->run_start) {
      m_data->run_start = false;
      run_start = false;
    } else if (boost::posix_time::microsec_clock::universal_time() >= start_time) {
      m_data->start_time = start_time;
      m_data->run_start  = true;
    };

  if (run_stop)
    if (m_data->run_stop) {
      m_data->run_stop = false;
      run_stop = false;
    } else
      m_data->run_stop = true;

  if (change_config)
    if (m_data->change_config) {
      m_data->change_config = false;
      change_config         = false;
    } else {
      m_data->run_configuration = run_configuration;
      m_data->change_config     = true;
    };

  return true;
};
