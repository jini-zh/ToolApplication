#include "RunControl.h"

RunControl_args::RunControl_args():Thread_args(){
  start_time=0;
  current_coarse_counter=0;
  
}

RunControl_args::~RunControl_args(){}


RunControl::RunControl():Tool(){}


bool RunControl::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();


  //put this in a load config funciton
  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("config_update_time_sec",m_config_update_time_sec)) m_config_update_time_sec=300;

  m_run_start=false;
  m_run_stop=false;
  m_start_time=&m_data->start_time;

  m_period_new_sub_run=boost::posix_time::hours(12);
  m_period_reconfigure=boost::posix_time::seconds(m_config_update_time_sec);
  
  m_util=new Utilities();
  args=new RunControl_args();

  m_data->run_number=0;
  m_data->sub_run_number=0;

  args->start_time=&m_data->start_time;
  args->current_coarse_counter=&m_data->current_coarse_counter;
 
  m_util->CreateThread("test", &Thread, args);

  m_data->sc_vars.Add("RunStop",BUTTON, std::bind(&RunControl::RunStop, this,  std::placeholders::_1));
  m_data->sc_vars["RunStop"]->SetValue(0);
  m_data->sc_vars.Add("RunStart",COMMAND, std::bind(&RunControl::RunStart, this,  std::placeholders::_1));
  m_data->sc_vars["SubRunStart"]->SetValue(0);
  m_data->sc_vars.Add("SubRunStart",BUTTON, std::bind(&RunControl::SubRun, this,  std::placeholders::_1));
  m_data->sc_vars["SubRubStart"]->SetValue(0);
  
  ExportConfiguration();
  
  return true;
}


bool RunControl::Execute(){

  if(m_data->change_config) InitialiseConfiguration();

  if(m_data->run_start) m_data->run_start=false;
  if(m_data->run_stop) m_data->run_stop=false;
  if(m_data->sub_run) m_data->sub_run=false;
  
  
  if(m_run_start){
    m_lapse = m_period_reconfigure -( boost::posix_time::microsec_clock::universal_time() - (m_config_start));

    if(m_lapse.is_negative() && !m_data->change_config){
      
      *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
      unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
      std::string json_payload="{\"Timestamp\":" + std::to_string(secs_since_epoch) + "}";
      m_data->sc_vars.AlertSend("RunStart",json_payload);
      std::stringstream sql_query;
      sql_query<<"insert into run_info values ((select max(run) from run_info) +1,0,"<<std::to_string(secs_since_epoch)<<",NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"');";
      m_data->services->SQLQuery("daq",sql_query.str());
      m_data->sub_run_number=0;
      m_data->run_number=1;
	
      
      //time_t now = time(0);
      //struct tm y2k = {0};
      //tm utc = *gmtime(&now);
      //*m_start_time= difftime(mktime(&utc),mktime(&y2k));
      //update DB start time;
      //  m_data->start_time= *m_start_time;
      
      m_data->run_start=true;
      m_run_start=false;  
    }
  }
  
  if(m_run_stop){
    m_data->run_stop=true;
    m_run_stop=false;
  }
  if(m_new_sub_run) m_data->sub_run;
  
  m_lapse = m_period_new_sub_run -( boost::posix_time::microsec_clock::universal_time() - (*m_start_time));
  if(m_lapse.is_negative()) SubRun("");
  
  usleep(100);
     
  return true;
}


bool RunControl::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  m_start_time=0;

  return true;
}

void RunControl::Thread(Thread_args* arg){
  
  RunControl_args* args=reinterpret_cast<RunControl_args*>(arg);
  
  boost::posix_time::time_duration td = (boost::posix_time::microsec_clock::universal_time() - *(args->start_time));

  *(args->current_coarse_counter)=td.total_milliseconds()*125000;

  usleep(1000);
  
}

std::string RunControl::RunStart(const char* key){

  
  m_data->load_config=true;
  std::string run_json="";
  m_data->sc_vars["RunStart"]->GetValue(run_json);
  Store run_info;
  run_info.JsonParser(run_json);
  if(!run_info.Get("run_description",m_run_description)) m_run_description="NONE";
  if(!run_info.Get("run_configuration",m_data->run_configuration)){

    //throw an error or something
  }
  
  
  std::string json_payload="{\"RunConfig\":" + std::to_string(m_data->run_configuration) + "}";
  m_data->sc_vars.AlertSend("ChangeConfig", json_payload);
  m_run_start=true;
  
  m_config_start=boost::posix_time::microsec_clock::universal_time();

  return "new Run started";
  
}

std::string RunControl::RunStop(const char* key){
  
  m_data->sc_vars.AlertSend("RunStop");     
  m_run_stop=true;
  std::stringstream sql_query;
  sql_query<<"update run_info set stop_time = now() where run_number = "<< m_data->run_number<<" and sub_run_number = "<<m_data->sub_run_number; //maybenot now maybe local ptime
  m_data->services->SQLQuery("daq",sql_query.str());

  return "Run stopped";
  
}

std::string RunControl::SubRun(const char* key){
  
  m_new_sub_run=true;     
      
  *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
  unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
  
  std::stringstream sql_query;
  sql_query<<"insert into run_info values ((select max(run) from run_info),(select max(sub_run) from run_info) +1,"<<std::to_string(secs_since_epoch)<<",NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"');";
  m_data->services->SQLQuery("daq",sql_query.str());
  m_data->sub_run_number=0;
  m_data->run_number=1;
  
  
  //update db
  //update local variable and start time
  
  return "Run stopped";
  
}
