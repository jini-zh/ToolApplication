#include "HKMPMT.h"

HKMPMT_args::HKMPMT_args():Thread_args(){}

HKMPMT_args::~HKMPMT_args(){}


HKMPMT::HKMPMT():Tool(){}


bool HKMPMT::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new HKMPMT_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool HKMPMT::Execute(){

  return true;
}


bool HKMPMT::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void HKMPMT::Thread(Thread_args* arg){

  HKMPMT_args* args=reinterpret_cast<HKMPMT_args*>(arg);

}
