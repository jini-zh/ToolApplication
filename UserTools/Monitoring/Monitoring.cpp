#include "Monitoring.h"

Monitoring_args::Monitoring_args():Thread_args(){}

Monitoring_args::~Monitoring_args(){}


Monitoring::Monitoring():Tool(){}


bool Monitoring::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new Monitoring_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Monitoring::Execute(){

  return true;
}


bool Monitoring::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void Monitoring::Thread(Thread_args* arg){

  Monitoring_args* args=reinterpret_cast<Monitoring_args*>(arg);

}
