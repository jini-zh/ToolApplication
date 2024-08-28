#include "JobManager.h"

JobManager::JobManager():Tool(){}


bool JobManager::Initialise(std::string configfile, DataModel &data){
  
  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

 if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
 if(!m_variables.Get("tread_cap",m_thread_cap)) m_thread_cap=4;
 if(!m_variables.Get("global_tread_cap",m_data->thread_cap)) m_data->thread_cap=4;
 
 m_data->thread_num=0;
 
 worker_pool_manager= new WorkerPoolManager(m_data->job_queue, &m_thread_cap, &(m_data->thread_cap), &(m_data->thread_num));
 
 
 ExportConfiguration();
 
 return true;
}


bool JobManager::Execute(){

  return true;
}


bool JobManager::Finalise(){

  delete worker_pool_manager;
  worker_pool_manager=0;
  
  return true;
}
