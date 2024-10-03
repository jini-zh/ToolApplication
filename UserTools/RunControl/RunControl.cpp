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
  
  // printf("d1\n");
  
  LoadConfig();
  //printf("d2\n");
  
  m_run_start=false;
  m_run_stop=false;
  m_start_time=&m_data->start_time;
  //printf("d3\n");
  
  m_util=new Utilities();
  args=new RunControl_args();
  //printf("d4\n");
  
  m_data->run_number=0;
  m_data->sub_run_number=0;
  //printf("d5\n");
  
  args->start_time=&m_data->start_time;
  args->current_coarse_counter=&m_data->current_coarse_counter;
  //printf("d6\n");
  
  m_util->CreateThread("test", &Thread, args);
  //printf("d7\n");
  
  m_data->sc_vars.Add("RunStop",BUTTON, std::bind(&RunControl::RunStop, this,  std::placeholders::_1));
  //printf("d7.1\n");
  m_data->sc_vars["RunStop"]->SetValue(0);
  //printf("d7.2\n");
  m_data->sc_vars.Add("RunStart",COMMAND, std::bind(&RunControl::RunStart, this,  std::placeholders::_1));
  //printf("d7.3\n");
  m_data->sc_vars["RunStart"]->SetValue(0);
  //printf("d7.4\n");
  m_data->sc_vars.Add("SubRunStart",BUTTON, std::bind(&RunControl::SubRun, this,  std::placeholders::_1));
  //printf("d7.5\n");
  m_data->sc_vars["SubRunStart"]->SetValue(0);
  //printf("d8\n");
  
  ExportConfiguration();
  //printf("d9\n");
  
  m_data->running=false;
  
  return true;
}


bool RunControl::Execute(){

  if(m_data->change_config){
    InitialiseConfiguration();
    LoadConfig();
  }
  if(m_data->run_start) m_data->run_start=false;
  if(m_data->run_stop) m_data->run_stop=false;
  if(m_data->sub_run) m_data->sub_run=false;
  
  
  if(m_run_start){
    //    printf("in runstart\n");
    m_lapse = m_period_reconfigure -( boost::posix_time::microsec_clock::universal_time() - (m_config_start));
    //std::cout<< m_lapse<<std::endl;
    if(m_lapse.is_negative() && !m_data->change_config){
      //printf("in runstart lapse\n");
      
      *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
      unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
      std::string json_payload="{\"Timestamp\":" + std::to_string(secs_since_epoch) + "}";
      m_data->sc_vars.AlertSend("RunStart",json_payload);
      m_data->running=true;
      std::stringstream sql_query;
      std::string sql_respose="";
      sql_query<<"select max(run) from run_info;";
      m_data->services->SQLQuery("daq",sql_query.str(),sql_respose);
      // std::cout<<"sql response="<<sql_respose<<std::endl;
      Store response_Store;
      response_Store.JsonParser(sql_respose);
      unsigned int run_num=0;
      response_Store.Get("max",run_num);
      run_num++;      
      sql_query<<"insert into run_info values ("<<run_num<<",0,TIMEZONE('UTC', TO_TIMESTAMP("<<std::to_string(secs_since_epoch)<<")),NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"');";
      m_data->services->SQLQuery("daq",sql_query.str());
      m_data->sub_run_number=0;
      m_data->run_number=run_num;
	
      
      //time_t now = time(0);
      //struct tm y2k = {0};
      //tm utc = *gmtime(&now);
      //*m_start_time= difftime(mktime(&utc),mktime(&y2k));
      //update DB start time;
      //  m_data->start_time= *m_start_time;
      
      m_data->run_start=true;
      m_run_start=false;
      std::stringstream tmp;
      tmp<<"R"<<m_data->run_number<<"S"<<m_data->sub_run_number; 
      m_data->vars.Set("Status", tmp.str());

      // printf("runstart lapse end\n");
    }
  }
  
  if(m_run_stop){
    m_data->vars.Set("Status", "Run Stopped");
    m_data->run_stop=true;
    m_run_stop=false;
  }
  if(m_new_sub_run){
    std::stringstream tmp;
    tmp<<"R"<<m_data->run_number<<"S"<<m_data->sub_run_number; 
    m_data->vars.Set("Status", tmp.str());

    m_data->sub_run=true;
    m_new_sub_run=false;
  }
  
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
  if(m_run_start){
    m_data->sc_vars["RunStart"]->SetValue("command");
    return "Error: Already Starting new run";
  }
  if(m_data->running) RunStop("");
  //  printf("hello : %s\n", key);
  std::string run_json="";
  m_data->sc_vars["RunStart"]->GetValue(run_json);
  m_data->sc_vars["RunStart"]->SetValue("command");
  Store run_info;
  run_info.JsonParser(run_json);
  //run_info.Print();
  if(!run_info.Get("run_description",m_run_description)) m_run_description="NONE";
  if(!run_info.Get("run_configuration",m_data->run_configuration)){
    m_data->services->SendLog("ERROR DAQ Configuration: No run_configuration received from Web" , 0);
    m_data->services->SendAlarm("ERROR DAQ Configuration: No run_configuration received from Web");
    return "Error with web run configuration";
  }
  
  m_data->load_config=true; 
  
  std::string json_payload="{\"RunConfig\":" + std::to_string(m_data->run_configuration) + "}";
  m_data->sc_vars.AlertSend("ChangeConfig", json_payload);
  m_run_start=true;
  m_data->vars.Set("Status", "Starting new run");
  m_config_start=boost::posix_time::microsec_clock::universal_time();

  return "new Run started";
  
}

std::string RunControl::RunStop(const char* key){

  if(!m_data->running) return "ERROR: Detector not running";
  if(key!="N"){
    m_data->running=false;
    m_data->sc_vars.AlertSend("RunStop");     
    m_run_stop=true;
  }
  std::stringstream sql_query;
  sql_query<<"update run_info set stop_time = now() where run = "<< m_data->run_number<<" and subrun = "<<m_data->sub_run_number; //maybenot now maybe local ptime
  m_data->services->SQLQuery("daq",sql_query.str());

  return "Run stopped";
  
}

std::string RunControl::SubRun(const char* key){

  if(!m_data->running) return "Error: Detector Not Running";
  RunStop("N");
  
  m_new_sub_run=true;     
      
  *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
  unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
  m_data->sub_run_number++;  //maybe make this better using definite DB values
  
  std::stringstream sql_query;
  sql_query<<"insert into run_info values ("<<m_data->run_number<<","<<m_data->sub_run_number<<",TIMEZONE('UTC', TO_TIMESTAMP("<<std::to_string(secs_since_epoch)<<")),NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"');";
  m_data->services->SQLQuery("daq",sql_query.str());
  
  
  //update db
  //update local variable and start time
  
  return "New SubRun started";
  
}

void RunControl::LoadConfig(){
  
  //put this in a load config funciton
  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("config_update_time_sec",m_config_update_time_sec)) m_config_update_time_sec=30;
  
  m_period_new_sub_run=boost::posix_time::hours(12);
  m_period_reconfigure=boost::posix_time::seconds(m_config_update_time_sec);
  
  
}
