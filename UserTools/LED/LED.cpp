#include "LED.h"

LED_args::LED_args():Thread_args(){}

LED_args::~LED_args(){}


LED::LED():Tool(){}


bool LED::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new LED_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool LED::Execute(){

  return true;
}


bool LED::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void LED::Thread(Thread_args* arg){

  LED_args* args=reinterpret_cast<LED_args*>(arg);

}
