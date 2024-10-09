#include "RunControl.h"

RunControl_args::RunControl_args():Thread_args(){
  start_time=0;
  current_coarse_counter=0;
  
}

RunControl_args::~RunControl_args(){}


RunControl::RunControl():Tool(){}


bool RunControl::Initialise(std::string configfile, DataModel &data){
  
  InitialiseTool(data);
  
  m_configfile = configfile;
  InitialiseConfiguration(m_configfile);
  ExportConfiguration();
  
  Log(m_tool_name+" initialised configuration",v_debug,m_verbose);
  std::cout<<m_tool_name<<" m_variables:\n--------------\n";
  m_variables.Print();
  std::cout<<"--------------\n";
  
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
  
  // FIXME better name than 'test'
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
    
    InitialiseConfiguration(m_configfile);
    LoadConfig();
    ExportConfiguration();
    std::cout<<m_tool_name<<" m_variables:\n--------------\n";
    m_variables.Print();
    std::cout<<"--------------\n";
    
  }

  if(m_data->run_start) m_data->run_start=false;
  if(m_data->run_stop) m_data->run_stop=false;
  if(m_data->sub_run) m_data->sub_run=false;
  
  
  if(m_run_start){
    //    printf("in runstart\n");
    try{
	    m_lapse = m_period_reconfigure -( boost::posix_time::microsec_clock::universal_time() - (m_config_start));
           //std::cout<< m_lapse<<std::endl;

	    if(m_lapse.is_negative() && !m_data->change_config){
              //printf("in runstart lapse\n");
	      
	      *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
	      unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
	      
	      std::string json_payload="{\"Timestamp\":" + std::to_string(secs_since_epoch) + "}";
	      bool ok = m_data->sc_vars.AlertSend("RunStart",json_payload);
	      if(!ok){
	        std::string errmsg = "ERROR "+m_tool_name+"::Execute failed to send RunStart alert with payload '"+json_payload+"'";
	        throw std::runtime_error(errmsg);
	      }
	      m_data->running=true;
	      
	      std::stringstream sql_query;
	      std::string response;
	      sql_query<<"insert into run_info values ((select COALESCE(max(run)+1,0) from run_info),0,TIMEZONE('UTC', TO_TIMESTAMP("<<std::to_string(secs_since_epoch)<<")),NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"') returning run;";
	      ok = m_data->services->SQLQuery("daq",sql_query.str(), response);
	      if(!ok){
	        // FIXME what do do about updating configs, sending an alert, and then finding an error trying to make new run DB entry?
	        std::string errmsg = "ERROR "+m_tool_name+"::Execute Failed to make run_info database entry for new run with response '"+response+"'";
	        throw std::runtime_error(errmsg);
	      }
	      
	      Store response_store;
	      response_store.JsonParser(response);
	      if(!response_store.Get("run",m_data->run_number)){
	        std::string errmsg = "ERROR "+m_tool_name+"::Execute failed to extract new run number from response '"+response+"'";
	        throw std::runtime_error(errmsg);
	      }
	      m_data->sub_run_number=0;
	      
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
              
              //m_data->services->SendLog("Run "+std::to_string(m_data->run_number)+" started", 0);
              Log("Run "+std::to_string(m_data->run_number)+" started", 0);
              
              // printf("runstart lapse end\n");
              
	    }
    } catch(std::exception& e){
        m_data->services->SendLog(e.what(), 0);
        m_data->services->SendAlarm(e.what());
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
    std::string errmsg = "ERROR "+m_tool_name+"::RunStart failed to get run_configuration for start of run from payload '"+run_json+"'";
//{"run_description": "new run", "run_configuration":0}
//failed to get run_configuration for start of run from payload '{'
    m_data->services->SendLog(errmsg, 0);
    m_data->services->SendAlarm(errmsg);
    return errmsg;
  }
  
  // send alert to inform other systems to update their configurations
  std::string json_payload="{\"RunConfig\":" + std::to_string(m_data->run_configuration) + "}";
  bool ok = m_data->sc_vars.AlertSend("ChangeConfig", json_payload);
  if(!ok){
    std::string errmsg = "ERROR "+m_tool_name+"::RunStart failed to send ChangeConfig alert with payload '"+json_payload+"'";
    m_data->services->SendLog(errmsg, 0);
    m_data->services->SendAlarm(errmsg);
    return errmsg;
  }
  
  m_run_start=true;
  m_data->load_config=true;
  
  m_config_start=boost::posix_time::microsec_clock::universal_time();

  return "new Run started";
  
}

std::string RunControl::RunStop(const char* key){

  if(!m_data->running) return "ERROR: Detector not running";
  if(key!="N"){
    m_data->sc_vars.AlertSend("RunStop");
    bool ok = m_data->sc_vars.AlertSend("RunStop");
    if(!ok){
      std::string errmsg = "ERROR "+m_tool_name+"::RunStop failed to send RunStop alert";
      m_data->services->SendLog(errmsg, 0);
      m_data->services->SendAlarm(errmsg);
      return errmsg;
    }
    m_data->running=false;
    m_run_stop=true;
  }
  
  std::stringstream sql_query;
  // FIXME maybe not now maybe local ptime
  sql_query<<"update run_info set stop_time = now() where run_number = "<< m_data->run_number<<" and sub_run_number = "<<m_data->sub_run_number;
  ok = m_data->services->SQLQuery("daq",sql_query.str());
  if(!ok){
    std::string errmsg = "ERROR "+m_tool_name+"::RunStop Failed to update end time of run with response '"+response+"'";
    m_data->services->SendLog(errmsg, 0);
    m_data->services->SendAlarm(errmsg);
    return errmsg;
  }
  
  //m_data->services->SendLog("Run "+std::to_string(m_data->run_number)+ " stopped", 0);
  Log("Run "+std::to_string(m_data->run_number)+ " stopped", 0);
  return "Run stopped";
  
}

std::string RunControl::SubRun(const char* key){

  if(!m_data->running) return "Error: Detector Not Running";
  RunStop("N");
  
  //update local variable and start time
  // FIXME if this function does not complete m_start_time will be incorrect, preventing automatic subrun rollover for another subrun period
  *m_start_time= boost::posix_time::microsec_clock::universal_time() +  boost::posix_time::minutes(1); ///now+1min
  unsigned long secs_since_epoch= boost::posix_time::time_duration(*m_start_time -  boost::posix_time::time_from_string("1970-01-01 00:00:00.000")).total_seconds();
  
  // FIXME maybe make this better using definite DB values
  m_data->sub_run_number++;
  
  //update db
  std::stringstream sql_query;
  sql_query<<"insert into run_info values ("<<m_data->run_number<<","<<m_data->sub_run_number<<",TIMEZONE('UTC', TO_TIMESTAMP("<<std::to_string(secs_since_epoch)<<")),NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"');";
  //sql_query<<"insert into run_info values ( (SELECT MAX(run) FROM run_info), ((SELECT MAX(subrun) FROM run_info WHERE run=(SELECT MAX(run) FROM run_info))+1), TIMEZONE('UTC', TO_TIMESTAMP("<<std::to_string(secs_since_epoch)<<")),NULL,"<<m_data->run_configuration<<",'"<<m_run_description<<"') returning run,subrun;";
  
  std::string response;
  bool ok = m_data->services->SQLQuery("daq",sql_query.str(),response);
  if(!ok){
    std::string errmsg = "ERROR "+m_tool_name+"::SubRun Failed to make database entry for new subrun with response '"+response+"'";
    m_data->services->SendLog(errmsg, 0);
    m_data->services->SendAlarm(errmsg);
    return errmsg;
  }
  Store response_store;
  response_store.JsonParser(response);
  if(!response_store.Get("run",m_data->run_number)){
      std::string errmsg = "ERROR "+m_tool_name+"::SubRun failed to extract new run number from response '"+response+"'";
      throw std::runtime_error(errmsg);
  }
  if(!response_store.Get("subrun",m_data->sub_run_number)){
      std::string errmsg = "ERROR "+m_tool_name+"::SubRun failed to extract new subrun number from response '"+response+"'";
      throw std::runtime_error(errmsg);
  }
  
  m_new_sub_run=true;
  
  m_data->services->SendLog("Run "+std::to_string(m_data->run_number)+ " SubRun "+std::to_string(m_data->sub_run_number)+" started", 0);
  return "New SubRun started";
  
}

void RunControl::LoadConfig(){
  
  //put this in a load config funciton
  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  if(!m_variables.Get("config_update_time_sec",m_config_update_time_sec)) m_config_update_time_sec=30;
  
  m_period_new_sub_run=boost::posix_time::hours(12);
  m_period_reconfigure=boost::posix_time::seconds(m_config_update_time_sec);
  
}
