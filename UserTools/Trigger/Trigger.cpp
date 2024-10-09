#include "Trigger.h"

Trigger_args::Trigger_args():Thread_args(){
  data=0;
}

Trigger_args::~Trigger_args(){
  data=0;
}


Trigger::Trigger():Tool(){}


bool Trigger::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new Trigger_args();
  args->data=m_data; 
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Trigger::Execute(){

  // laod configuration on config change;
  
  return true;
}


bool Trigger::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void Trigger::Thread(Thread_args* arg){

  Trigger_args* args=reinterpret_cast<Trigger_args*>(arg);

  if(!args->data->sorted_data.size()){
    usleep(100);
    return;
  }
  std::map<unsigned int, MPMTData*> sorted_data;
  args->data->sorted_data_mtx.lock();
  std::swap(sorted_data , args->data->sorted_data);
  args->data->sorted_data_mtx.unlock();
   
  for(std::map<unsigned int, MPMTData*>::iterator it= sorted_data.begin(); it!= sorted_data.end(); it++){
    
    Job* tmp_job = new Job("triggering");
    Trigger_args*  tmp_args= new Trigger_args;
    tmp_args->data = args->data;
    tmp_args->bin = it->first;
    tmp_args->sorted_data = it->second;
    
    for(int j=0; j<args->triggers.size(); j++){
      tmp_args->trigger_functions.push_back(args->data->trigger_functions[args->triggers.at(j)]);
      Trigger_algo_args* tmp_algo_args = new Trigger_algo_args;
      tmp_algo_args->m_data = args->data;
      tmp_algo_args->sorted_data = it->second;
      tmp_algo_args->trigger_vars = args->data->trigger_vars[args->triggers.at(j)];
      args->trigger_algo_args.push_back(tmp_algo_args);
    }
    
    tmp_job->func=TriggerJob;
    args->data->job_queue.AddJob(tmp_job);

  }
  
}

bool Trigger::TriggerJob(void* data){

Trigger_args* args=reinterpret_cast<Trigger_args*>(args);

 for(int i=0; i<args->trigger_functions.size(); i++){

   args->trigger_functions.at(i)(args->trigger_algo_args.at(i));

 }

 args->data->triggered_data_mtx.lock();
 args->data->triggered_data[args->bin] = args->sorted_data;
 args->data->triggered_data_mtx.unlock();

 
 return true;

}
