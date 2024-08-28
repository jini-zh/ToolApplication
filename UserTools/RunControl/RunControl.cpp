#include "RunControl.h"

RunControl_args::RunControl_args():Thread_args(){}

RunControl_args::~RunControl_args(){}


RunControl::RunControl():Tool(){}


bool RunControl::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("config_update_time_sec",m_config_update_time_sec)) m_config_update_time_sec=300;

  m_run_start=false;
  m_run_stop=false;
  m_start_time=0;
  
  //m_util=new Utilities();
  //args=new RunControl_args();
  
  //  m_util->CreateThread("test", &Thread, args);


  m_data->sc_vars.Add("RunStop",BUTTON, std::bind(&RunControl::RunStop, this,  std::placeholders::_1));
  m_data->sc_vars["RunStop"]->SetValue(0);
  m_data->sc_vars.Add("RunStart",VARIABLE, std::bind(&RunControl::RunStart, this,  std::placeholders::_1));
  m_data->sc_vars["RunStart"]->SetValue(0);  
  
  ExportConfiguration();
  
  return true;
}


bool RunControl::Execute(){

  if(m_data->change_config) InitialiseConfiguration("");

  if(m_data->run_start) m_data->run_start=false;
  if(m_data->run_stop) m_data->run_stop=false;
  
  if(m_run_start){
    m_data->run_start=true;
    m_run_start=false;  
  }
  if(m_run_stop){
    m_data->run_stop=true;
    m_run_stop=false;
  }

  usleep(100);
  return true;
}


bool RunControl::Finalise(){

  //  m_util->KillThread(args);

  //delete args;
  //args=0;

  //  delete m_util;
  //m_util=0;

  return true;
}

//void RunControl::Thread(Thread_args* arg){

//  RunControl_args* args=reinterpret_cast<RunControl_args*>(arg);

//}

std::string RunControl::RunStart(const char* key){

  ////////////this blocking is no good and needs to be corrected. have to farm off the wait to the execture process.
  
  m_data->load_config=true;
  unsigned int run_configuration=0;
  m_data->sc_vars["RunStart"]->GetValue(run_configuration);
  //add new db entry to run table
  std::string json_payload="{\"RunConfig\":" + std::to_string(run_configuration) + "}";
  m_data->sc_vars.AlertSend("ChangeConfig", json_payload);
  sleep(m_config_update_time_sec);
  while(m_data->change_config) sleep(5);
  
  m_start_time= 1; // now + a min
  json_payload="{\"Timestamp\":" + std::to_string(m_start_time) + "}";
  m_data->sc_vars.AlertSend("RunStart");
  time_t now = time(0);
  struct tm y2k = {0};
  tm utc = *gmtime(&now);
  m_start_time= difftime(mktime(&utc),mktime(&y2k));
  //update DB start time;
  m_data->start_time= m_start_time;
  m_run_start=true;

  return "Run started at m_start_time";
  
}

std::string RunControl::RunStop(const char* key){
  
  m_data->sc_vars.AlertSend("RunStop");     
  m_run_stop=true;
 //update db stoptime in run table
  

  return "Run stopped";
  
}
