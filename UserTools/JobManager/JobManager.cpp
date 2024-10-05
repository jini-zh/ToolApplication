#include "JobManager.h"

JobManager::JobManager():Tool(){}


bool JobManager::Initialise(std::string configfile, DataModel &data){
  
  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

 if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
 if(!m_variables.Get("thread_cap",m_thread_cap)) m_thread_cap=14;
 if(!m_variables.Get("global_thread_cap",m_data->thread_cap)) m_data->thread_cap=14;

 bool self_serving =true;
 m_variables.Get("self_serving", self_serving);
 
 m_data->thread_num=0;
 
 worker_pool_manager= new WorkerPoolManager(m_data->job_queue, &m_thread_cap, &(m_data->thread_cap), &(m_data->thread_num), nullptr, self_serving);
 
 
 ExportConfiguration();
 
 return true;
}


bool JobManager::Execute(){

  m_data->monitoring_store.Set("pool_threads",worker_pool_manager->NumThreads());
  m_data->monitoring_store.Set("queued_jobs",m_data->job_queue.size());
  if(worker_pool_manager->NumThreads()==m_thread_cap)  m_data->services->SendLog("Warning: Worker Pool Threads Maxed" , 0); //make this a warning
  // std::cout<<"NumThreads="<<worker_pool_manager->NumThreads()<<std::endl;
  //sleep(1);
  return true;
}


bool JobManager::Finalise(){

  delete worker_pool_manager;
  worker_pool_manager=0;
  
  return true;
}
