#include "Configuration.h"

Configuration_args::Configuration_args():Thread_args(){}

Configuration_args::~Configuration_args(){}


Configuration::Configuration():Tool(){}


bool Configuration::Initialise(std::string configfile, DataModel &data){

  
  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();
  
  LoadConfig();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  
  m_data->run_configuration=0;  
  m_data->change_config=false;
  m_data->load_config=false;
  
  //  m_util=new Utilities();
  // args=new Configuration_args();
  
  //m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Configuration::Execute(){

  if(m_data->change_config)  m_data->change_config=false;
  
  if(m_data->load_config){

    LoadConfig();
    
    m_data->load_config=false;
  }
    

  return true;
}


bool Configuration::Finalise(){

  //m_util->KillThread(args);

  //  delete args;
  //args=0;

  //delete m_util;
  //m_util=0;

  return true;
}

void Configuration::Thread(Thread_args* arg){

  Configuration_args* args=reinterpret_cast<Configuration_args*>(arg);

}

bool Configuration::LoadConfig(){

  std::string config_json="";
  if(m_data->services->GetRunDeviceConfig(config_json, m_data->run_configuration)){
    m_data->vars.JsonParser(config_json);
    m_data->change_config=true;
    InitialiseConfiguration("");
    return true;
  }
    m_data->services->SendLog("ERROR DAQ Configuration: Failed to load config from DB" , 0);
    m_data->services->SendAlarm("ERROR DAQ Configuration: Failed to load config from DB");
    return false;
}
