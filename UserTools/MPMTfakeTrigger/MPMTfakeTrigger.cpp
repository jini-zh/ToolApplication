#include "MPMTfakeTrigger.h"

MPMTfakeTrigger_args::MPMTfakeTrigger_args():Thread_args(){}

MPMTfakeTrigger_args::~MPMTfakeTrigger_args(){}


MPMTfakeTrigger::MPMTfakeTrigger():Tool(){}


bool MPMTfakeTrigger::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new MPMTfakeTrigger_args();
  args->data=m_data;
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool MPMTfakeTrigger::Execute(){

  return true;
}


bool MPMTfakeTrigger::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void MPMTfakeTrigger::Thread(Thread_args* arg){

  MPMTfakeTrigger_args* args=reinterpret_cast<MPMTfakeTrigger_args*>(arg);

  
}
