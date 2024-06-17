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
