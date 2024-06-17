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
  *m_log << ML(3) << "success" << std::endl;
};

void V792::readout() {
  qdc->readout(buffer);
  if (buffer.size() == 0) {
    usleep(100);
    return;
  };

  std::vector<caen::V792::Packet> data(buffer.begin(), buffer.end());

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
