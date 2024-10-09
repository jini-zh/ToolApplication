#include "Monitoring.h"

Monitoring_args::Monitoring_args():Thread_args(){}

Monitoring_args::~Monitoring_args(){}


Monitoring::Monitoring():Tool(){}


bool Monitoring::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();



  m_util=new Utilities();
  args=new Monitoring_args();
  args->last =  boost::posix_time::microsec_clock::universal_time();

  LoadConfig();

  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Monitoring::Execute(){

  if(m_data->change_config){
    InitialiseConfiguration();
    LoadConfig();
  }
  
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
  
  args->lapse = args->period -( boost::posix_time::microsec_clock::universal_time() - args->last);
  //std::cout<< m_lapse<<std::endl;
  
  if(!args->lapse.is_negative() ){
    usleep(100);
    return;
  }
    //printf("in runstart lapse\n");
    
    std::string json="";
    args->data->monitoring_store_mtx.lock();
    args->data->monitoring_store>>json;
    args->data->monitoring_store_mtx.unlock();
    args->services->SendMonitoringData(json);
    
    args->last = boost::posix_time::microsec_clock::universal_time();
    
    
}

bool Monitoring::LoadConfig(){

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  unsigned int period_sec=0;
  if(!m_variables.Get("period_sec",period_sec)) period_sec=120;
  args->period = boost::posix_time::seconds(period_sec);
  
  
  return true;
  
}
