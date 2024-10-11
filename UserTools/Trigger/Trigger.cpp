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

  m_data->trigger_functions["beam"]=BeamTrigger;
  m_data->trigger_functions["led"]=LedTrigger;
  m_data->trigger_functions["none"]=NoneTrigger;

  m_data->trigger_vars["beam"]=&beam_vars;
  m_data->trigger_vars["led"]=&led_vars;
  m_data->trigger_vars["none"]=&none_vars;
  
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

bool Trigger::BeamTrigger(void* data){

Trigger_algo_args* args=reinterpret_cast<Trigger_algo_args*>(data);

 for(unsigned int i=0; i<args->sorted_data->mpmt_triggers.size(); i++){
    TriggerInfo tmp_trigger;
   switch(args->sorted_data->mpmt_triggers.at(i).GetChannel()){
   case 0: //pre-spill warning
     args->m_data->spill_num++;
     
     break;
   case 1: //beam-spill
     
     break;
   case 2: //beam monitor trigger
     tmp_trigger.type = TriggerType::BEAM;
     tmp_trigger.time = ((unsigned long)args->sorted_data->coarse_counter << 48) |  ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetCoarseCounter() << 32) | ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetFineTime()); //this needs to be done properly into 64 bit almost certainly wrong
     tmp_trigger.spill_num = args->m_data->spill_num;
     tmp_trigger.vme_event_num = args->m_data->vme_event_num;       
     args->m_data->vme_event_num++;
     args->sorted_data->unmerged_triggers.push_back(tmp_trigger);
   
     break;
   case 3: //beam monitor - muon
     tmp_trigger.type = TriggerType::MBEAM;
     tmp_trigger.time = ((unsigned long)args->sorted_data->coarse_counter << 48) |  ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetCoarseCounter() << 32) | ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetFineTime()); //this needs to be done properly into 64 bit almost certainly wrong
     args->sorted_data->unmerged_triggers.push_back(tmp_trigger);
     
     break;
   case 4: //beam monitor - electron
     tmp_trigger.type = TriggerType::EBEAM;
     tmp_trigger.time = ((unsigned long)args->sorted_data->coarse_counter << 48) |  ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetCoarseCounter() << 32) | ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetFineTime()); //this needs to be done properly into 64 bit almost certainly wrong
     args->sorted_data->unmerged_triggers.push_back(tmp_trigger);
     
     break;
   case 5: //CDS laser ball pre-flash
     
     break;
   case 6: //hardware trigger
     tmp_trigger.type = TriggerType::HARD6;
     tmp_trigger.time = ((unsigned long)args->sorted_data->coarse_counter << 48) |  ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetCoarseCounter() << 32) | ((unsigned long) args->sorted_data->mpmt_triggers.at(i).GetFineTime()); //this needs to be done properly into 64 bit almost certainly wrong
     args->sorted_data->unmerged_triggers.push_back(tmp_trigger);

     
     break;
   }  
     

 }
 return true;
 
}

bool Trigger::LedTrigger(void* data){

  Trigger_algo_args* args=reinterpret_cast<Trigger_algo_args*>(data);

  return true;
}

bool Trigger::NoneTrigger(void* data){

  Trigger_algo_args* args=reinterpret_cast<Trigger_algo_args*>(data);


  return true;
}
