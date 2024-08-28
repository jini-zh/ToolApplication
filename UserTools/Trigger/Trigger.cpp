#include "Trigger.h"

Trigger_args::Trigger_args():Thread_args(){}

Trigger_args::~Trigger_args(){}


Trigger::Trigger():Tool(){}


bool Trigger::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new Trigger_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Trigger::Execute(){

  return true;
}


bool Trigger::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void Trigger::Thread(Thread_args* arg){

  Trigger_args* args=reinterpret_cast<Trigger_args*>(arg);

}
