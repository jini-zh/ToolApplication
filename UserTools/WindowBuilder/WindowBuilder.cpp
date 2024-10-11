#include "WindowBuilder.h"

WindowBuilder_args::WindowBuilder_args():Thread_args(){}

WindowBuilder_args::~WindowBuilder_args(){}


WindowBuilder::WindowBuilder():Tool(){}


bool WindowBuilder::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new WindowBuilder_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool WindowBuilder::Execute(){
  
  return true;
}


bool WindowBuilder::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void WindowBuilder::Thread(Thread_args* arg){

  WindowBuilder_args* args=reinterpret_cast<WindowBuilder_args*>(arg);

  if(args->data->triggered_data.count(0) && args->data->triggered_data.count(1)){



    //do stuff

    //   delete 0
  }

  std::vector<unsigned int> erase_list;
  
  for(std::map<unsigned int, MPMTData*>::iterator it=args->data->triggered_data.begin(); it!=args->data->triggered_data.end(); it++){


    
    if(args->data->triggered_data.count(it->first -1) && args->data->triggered_data.count(it->first +1)){


      //////////////////merging triggers//////////////////////////
      ///  think i need to temporally sort these triggers first PLEASE DO THIS BEN!!!!!!!
      
      std::vector<std::vector< unsigned int> >merged_triggers;
      std::map<unsigned int, bool> trigger_veto;
      
      for(unsigned int i=0; i<it->second->unmerged_triggers.size(); i++){
	if(!trigger_veto.count(i)){	
	  std::vector< unsigned int> tmp;
	  tmp.push_back(i);
	  
	  for(unsigned int j=i; j<it->second->unmerged_triggers.size(); j++){
	    
	    if(it->second->unmerged_triggers.at(j).time > ( it->second->unmerged_triggers.at(i).time + args->offset_trigger[it->second->unmerged_triggers.at(i).type] - args->pre_trigger[it->second->unmerged_triggers.at(i).type] ) && it->second->unmerged_triggers.at(j).time < ( it->second->unmerged_triggers.at(i).time + args->offset_trigger[it->second->unmerged_triggers.at(i).type] + args->post_trigger[it->second->unmerged_triggers.at(i).type] ) ){
	      tmp.push_back(j);
	      trigger_veto[j]=true;
	      
	      
	    }     
	  }
	    merged_triggers.push_back(tmp);
	  
	}
      }

    
    //////////////////////////////////////////////////

    //////////////////////////// collecting data////////////////////////

    //if() // time is out of bounds
      for(unsigned int i=0; i<merged_triggers.size(); i++){

	unsigned long max_time=0;
	unsigned long min_time=-1;
	ReadoutWindow* tmp_readout = new ReadoutWindow;

	//////////////////// collecting trigger_info//////////////
	for(unsigned int j=0; j<merged_triggers.at(i).size(); j++){  
	  tmp_readout->triggers_info.push_back(it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)));
	  if( it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).time + args->offset_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] - args->pre_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] <min_time) min_time = it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).time + args->offset_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] + args->pre_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type]; 
	  if( it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).time + args->offset_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] + args->post_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] > max_time) max_time = it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).time + args->offset_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type] + args->post_trigger[ it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type];
      
	}
	
	tmp_readout->start_counter=min_time;
	tmp_readout->readout_num=args->data->readout_num;
	args->data->readout_num++;
	/////////////////////////////////////////////

	///////////////////////// collecting mpmt_hits//////////////////////
	std::vector<WCTEMPMTHit>::iterator start_hit;
	std::vector<WCTEMPMTHit>::iterator stop_hit;
	unsigned int size=0;
	
	for(std::vector<WCTEMPMTHit>::iterator it_hit=it->second->mpmt_hits.begin(); it_hit!=it->second->mpmt_hits.end(); it_hit++){
	  if(it_hit->GetCoarseCounter() > max_time) break;
	  if(it_hit->GetCoarseCounter() > min_time){
	    if(it_hit->GetCoarseCounter() < start_hit->GetCoarseCounter()) start_hit= it_hit;
	    if(it_hit->GetCoarseCounter() > stop_hit->GetCoarseCounter()) stop_hit= it_hit;
	    size++;
	  }
	}

	tmp_readout->mpmt_hits.resize(size);
	std::memcpy(tmp_readout->mpmt_hits.data(), it->second->mpmt_hits.data(), sizeof(it->second->mpmt_hits.data()));
	/////////////////////////////////////////////////////////////////////////////

	///////////////////////// collecting mpmt_waveforms//////////////////////
	std::vector<WCTEMPMTWaveform>::iterator start_waveform;
	std::vector<WCTEMPMTWaveform>::iterator stop_waveform;
	size=0;
	
	for(std::vector<WCTEMPMTWaveform>::iterator it_waveform=it->second->mpmt_waveforms.begin(); it_waveform!=it->second->mpmt_waveforms.end(); it_waveform++){
	  if(it_waveform->header.GetCoarseCounter() > max_time) break;
	  if(it_waveform->header.GetCoarseCounter() > min_time){
	    if(it_waveform->header.GetCoarseCounter() < start_waveform->header.GetCoarseCounter()) start_waveform= it_waveform;
	    if(it_waveform->header.GetCoarseCounter() > stop_waveform->header.GetCoarseCounter()) stop_waveform= it_waveform;
	    size++;
	  }
	}
	
	tmp_readout->mpmt_waveforms.resize(size);
	std::memcpy(tmp_readout->mpmt_waveforms.data(), it->second->mpmt_waveforms.data(), sizeof(it->second->mpmt_waveforms.data()));
	/////////////////////////////////////////////////////////////////////////////
	  
	//////////////////////  collecting qdc and tdc hits  /////////////////////////
	for(unsigned int j=0; j<merged_triggers.at(i).size(); j++){
	  if(it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).type==TriggerType::BEAM){
	    unsigned long vme_event_vec_num=it->second->unmerged_triggers.at(merged_triggers.at(i).at(j)).vme_event_num - args->data->vme_poped;
	    // oh dear can only pop the front one BEN!!!! is this garenteed to be sequential in which case you can get rid of evt num etc.
	    tmp_readout->tdc_hits = args->data->tdc_readout.getEvent();
	    tmp_readout->qdc_hits = args->data->qdc_readout.getEvent();
	     
	  }
	  
	}
	

	//////////////////////////////////////////////////////////////////////////

	//adding readout window
	args->data->readout_windows_mtx.lock();
	args->data->readout_windows->push_back(tmp_readout);
	args->data->readout_windows_mtx.unlock();
	
      }
    
    if(it->first !=0) erase_list.push_back(it->first - 1); ///BEN!!!! THIS NEEDS CHECKING 
    
    }
  }

  /// deleting data;

  for (std::vector<unsigned int>::iterator it=erase_list.begin(); it!=erase_list.end(); it++){

    delete args->data->triggered_data[*it];
    args->data->triggered_data[*it]=0;

    args->data->triggered_data.erase(*it);

  }
  
  erase_list.clear();
  
}
