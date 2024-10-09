#include "Nhits.h"

Nhits::Nhits():Tool(){}


bool Nhits::Initialise(std::string configfile, DataModel &data){
  
  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  //your code here

  LoadVariables();
  
  m_data->trigger_functions["nhits"]=NhitsAlgo;
  m_data->trigger_vars["nhits"]=&m_trigger_vars;

  ExportConfiguration();

  return true;
}


bool Nhits::Execute(){
  
  if(m_data->change_config){
    InitialiseConfiguration();
    if(LoadVariables()){
      m_data->services->SendLog("ERROR: nhits configuration not correctly set" , 0);
      m_data->services->SendAlarm("ERROR: nhits configuration nto correctly set");
      return false;
    }
  }
  
  return true;
}


bool Nhits::Finalise(){

  return true;
}

bool Nhits::NhitsAlgo(void* data){

  Trigger_algo_args* args=reinterpret_cast<Trigger_algo_args*>(data);

  unsigned int threshold=0;
  unsigned int jump=0;
  unsigned int window_size=0;
  args->trigger_vars->Get("nhits_threshold", threshold); // maybe need to throw something here or return false // seems like this should be covered form the laod variables so maybe no longer needed.
  args->trigger_vars->Get("nhits_jump", jump);
  args->trigger_vars->Get("nhits_window_size", window_size);

  
  for(unsigned int i=0; i<(sizeof(args->sorted_data->cumulative_sum)/sizeof(unsigned int))-window_size; i++){
    if((args->sorted_data->cumulative_sum[i+window_size] - args->sorted_data->cumulative_sum[i]) >= threshold){
      TriggerInfo tmp;
      tmp.type=TriggerType::NHITS;
      tmp.time = ((((unsigned long) args->sorted_data->coarse_counter) << 32) & 0b1111111111111111111111111100000000000000000000000000000000000000) | (( ((unsigned long)i) << 13) & 0b0000000000000000000000000011111111111111111111111111111111111111);
      args->sorted_data->unmerged_triggers_mtx.lock();
      args->sorted_data->unmerged_triggers.push_back(tmp);
      args->sorted_data->unmerged_triggers_mtx.unlock();
      i+=jump;
    }
  }
  /*
  for(int i=0; i< times.size(); i++){
  TriggerInfo tmp;
  tmp.type=TriggerType::Nhits;
  tmp.time = ((coarse_counter << 32) & 0b1111111111111111111111111100000000000000000000000000000000000000) | (( time << 13) & 0b0000000000000000000000000011111111111111111111111111111111111111)
  //unsinged long upper = coarse_counter;
  //unsinged long lower = time;
  //  upper = (upper << 32);
  //lower = (lower << 13);
  //unsinged long time64= (upper & 0b1111111111111111111111111100000000000000000000000000000000000000) | (lower & 0b0000000000000000000000000011111111111111111111111111111111111111)
    
  */

  return true;

}


bool Nhits::LoadVariables(){

  
  if(!m_variables.Get("threshold", threshold)) return false;
   if(!m_variables.Get("jump", threshold)) return false;
   if(!m_variables.Get("window_size", threshold)) return false;
  
   m_trigger_vars.Set("nhits_threshold", threshold); 
   m_trigger_vars.Set("nhits_jump", jump);
   m_trigger_vars.Set("nhits_window_size", window_size); 

  return true;
}
