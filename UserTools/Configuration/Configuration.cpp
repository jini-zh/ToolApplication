#include "Configuration.h"

Configuration_args::Configuration_args():Thread_args(){}

Configuration_args::~Configuration_args(){}


Configuration::Configuration():Tool(){}


bool Configuration::Initialise(std::string configfile, DataModel &data){
  
  InitialiseTool(data);
  
  m_configfile=configfile;
  
  // retrieve configuration from database
  // update Tool configuration and export to datamodel
  // FIXME where is this run_configuration normally obtained?
  m_data->run_configuration=6;
  if(!LoadConfig()) return false;
  
  //  LoadConfig();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  
  m_data->change_config=false;
  m_data->load_config=false;
  
  //  m_util=new Utilities();
  // args=new Configuration_args();
  
  //m_util->CreateThread("test", &Thread, args);

  return true;
}


bool Configuration::Execute(){

  if(m_data->change_config)  m_data->change_config=false;
  
  if(m_data->load_config){

    if(!LoadConfig()) return false;
    
    m_data->load_config=false;
    m_data->change_config=true;
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

  //m_data->services->SendLog("Loading configuration "+std::to_string(m_data->run_configuration)+" from DB" , 1);
  Log("Loading configuration "+std::to_string(m_data->run_configuration)+" from DB" , 1);
  std::string config_json="";
  std::string response="";
  if(m_data->services->GetRunDeviceConfig(config_json, m_data->run_configuration)){
    
    m_data->vars.JsonParser(config_json);
    
    InitialiseConfiguration(m_configfile);
    ExportConfiguration();
    std::cout<<m_tool_name<<" m_variables:\n--------------\n";
    m_variables.Print();
    std::cout<<"--------------\n";
    
    m_data->change_config=true;
    
  } else {
    
    m_data->services->SendLog("ERROR "+m_tool_name+": Failed to load config from DB with error '"+config_json+"'" , 0);
    std::clog<<"sending alarm of error getting DB config"<<std::endl;
    bool ok = m_data->services->SendAlarm("ERROR "+m_tool_name+": Failed to load config from DB with error '"+config_json+"'");
    std::clog<<"sendAlarm of error getting DB config returned "<<ok<<std::endl;
    return false;
    
  }
  
  return true;
}
