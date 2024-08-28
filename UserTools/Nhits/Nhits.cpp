#include "Nhits.h"

Nhits_args::Nhits_args():Thread_args(){}

Nhits_args::~Nhits_args(){}


Nhits::Nhits():Tool(){}


bool Nhits::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new Nhits_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Nhits::Execute(){

  return true;
}


bool Nhits::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void Nhits::Thread(Thread_args* arg){

  Nhits_args* args=reinterpret_cast<Nhits_args*>(arg);

}
