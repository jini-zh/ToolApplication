#include "Sorting.h"

Sorting_args::Sorting_args():Thread_args(){}

Sorting_args::~Sorting_args(){}


Sorting::Sorting():Tool(){}


bool Sorting::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new Sorting_args();
  args->data=m_data;
  args->period=boost::posix_time::seconds(2);
  args->last=boost::posix_time::microsec_clock::universal_time();
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool Sorting::Execute(){

  m_data->monitoring_store.Set("unsorted_data_size",m_data->unsorted_data.size());
  m_data->monitoring_store.Set("sorted_data_size",m_data->sorted_data.size());
  
  
  return true;
}


bool Sorting::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void Sorting::Thread(Thread_args* arg){

  Sorting_args* args=reinterpret_cast<Sorting_args*>(arg);

   args->lapse = args->period -( boost::posix_time::microsec_clock::universal_time() - args->last);
   if(!args->lapse.is_negative()){
     sleep(1);
     return;
   }

   args->last= boost::posix_time::microsec_clock::universal_time();

   std::vector<MPMTData*> data_to_sort;
   std::vector<unsigned int> items_to_remove;

   args->data->unsorted_data_mtx.lock();

   for(std::map<unsigned int, MPMTData*>::iterator it= args->data->unsorted_data.begin(); it!= args->data->unsorted_data.end(); it++){
     if(it->first< (args->data->current_coarse_counter - 3333)){
       items_to_remove.push_back(it->first);
       data_to_sort.push_back(it->second);
       it->second=0;
     }

   }

   for(int i=0; i<items_to_remove.size(); i++){
     args->data->unsorted_data.erase(items_to_remove.at(i));
   }
   args->data->unsorted_data_mtx.unlock();

   for(int i=0 ; i<data_to_sort.size(); i++){
     Job* tmp_job= new Job("sorting");
     Sorting_args* tmp_args = new Sorting_args;
     tmp_args->data=args->data;
     tmp_args->unsorted_data=data_to_sort.at(i);
     tmp_job->func=SortData;
     args->data->job_queue.AddJob(tmp_job);
   }
   
}

bool Sorting::SortData(void* data){
  
  Sorting_args* args=reinterpret_cast<Sorting_args*>(data);
  
  MPMTData* tmp= new MPMTData;
  tmp->coarse_counter=args->unsorted_data->coarse_counter;
  
  unsigned int arr_count[4194303U] = { 0 };
  unsigned int arr[33554431U] = { 0 };
  
  //////////sort mpmt hits ///////////////

  //  ((coarsecounter & 4194303U)<<3) | (fine >> 13)
  
  for(std::vector<WCTEMPMTHit>::iterator it=args->unsorted_data->mpmt_hits.begin(); it!= args->unsorted_data->mpmt_hits.end(); it++){
    arr[((it->GetCoarseCounter() & 4194303U)<<3) | (it->GetFineTime() >>13)]++;
  }
  for( unsigned int i=1; i<33554431U; i++){
    arr[i]+=arr[i-1];
    //    tmp->cumulative_sum[i]=arr[i];
  }

  std::copy(std::begin(arr), std::end(arr), std::begin(tmp->cumulative_sum));

  tmp->mpmt_hits.resize(args->unsorted_data->mpmt_hits.size());

  unsigned int bin=0;
  for(unsigned int i=0; i< args->unsorted_data->mpmt_hits.size(); i++){
    bin=((args->unsorted_data->mpmt_hits.at(i).GetCoarseCounter() & 4194303U)<<3) | (args->unsorted_data->mpmt_hits.at(i).GetFineTime() >>13);
    tmp->mpmt_hits.at(arr[bin]-1)=args->unsorted_data->mpmt_hits.at(i);
    arr[bin]--;
    
  }

    memset(arr, 0, sizeof(arr));

  /////////////////////////////

  ///////// sorting LEDS ////////
  
  
  for(std::vector<WCTEMPMTLED>::iterator it=args->unsorted_data->mpmt_leds.begin(); it!= args->unsorted_data->mpmt_leds.end(); it++){
    arr_count[(it->GetCoarseCounter() & 4194303U)]++;
  }
  for( unsigned int i=1; i<4194303U; i++){
    arr_count[i]+=arr_count[i-1];
  }

  tmp->mpmt_leds.resize(args->unsorted_data->mpmt_leds.size());

  bin=0;
  for(unsigned int i=0; i< args->unsorted_data->mpmt_leds.size(); i++){
    bin=(args->unsorted_data->mpmt_leds.at(i).GetCoarseCounter() & 4194303U);
    tmp->mpmt_leds.at(arr_count[bin]-1)=args->unsorted_data->mpmt_leds.at(i);
    arr_count[bin]--;
  }

  memset(arr_count, 0, sizeof(arr_count));

  ///////////////////////////////////////////


  ///////// sorting Waveforms ////////
  
  
  for(std::vector<WCTEMPMTWaveform>::iterator it=args->unsorted_data->mpmt_waveforms.begin(); it!= args->unsorted_data->mpmt_waveforms.end(); it++){
    arr_count[(it->header.GetCoarseCounter() & 4194303U)]++;
  }
  for( unsigned int i=1; i<4194303U; i++){
    arr_count[i]+=arr_count[i-1];
  }

  tmp->mpmt_waveforms.resize(args->unsorted_data->mpmt_waveforms.size());

  bin=0;
  for(unsigned int i=0; i< args->unsorted_data->mpmt_waveforms.size(); i++){
    bin=(args->unsorted_data->mpmt_waveforms.at(i).header.GetCoarseCounter() & 4194303U);
    tmp->mpmt_waveforms.at(arr_count[bin]-1)=args->unsorted_data->mpmt_waveforms.at(i);
    arr_count[bin]--;
  }

  memset(arr_count, 0, sizeof(arr_count));

  ///////////////////////////////////////////


  ///////// sorting PPS ////////
  
  /*  
  for(std::vector<WCTEMPMTPPS>::iterator it=args->unsorted_data->mpmt_pps.begin(); it!= args->unsorted_data->mpmt_pps.end(); it++){
    arr_count[(it->GetCoarseCounter() & 4194303U)]++;
  }
  for( unsigned int i=1; i<4194303U; i++){
    arr_count[i]+=arr_count[i-1];
  }

  tmp->mpmt_pps.resize(args->unsorted_data->mpmt_pps.size());

  bin=0;
  for(unsigned int i=0; i< args->unsorted_data->mpmt_pps.size(); i++){
    bin=(args->unsorted_data->mpmt_pps.at(i).GetCoarseCounter() & 4194303U);
    tmp->mpmt_pps.at(arr_count[bin]-1)=args->unsorted_data->mpmt_pps.at(i);
    arr_count[bin]--;
  }

  memset(arr_count, 0, sizeof(arr_count));
  */
  ///////////////////////////////////////////

  ///////// sorting Triggers ////////
  
  
  for(std::vector<WCTEMPMTHit>::iterator it=args->unsorted_data->mpmt_triggers.begin(); it!= args->unsorted_data->mpmt_triggers.end(); it++){
    arr[((it->GetCoarseCounter() & 4194303U)<<3) | (it->GetFineTime() >>13)]++;
  }
  for( unsigned int i=1; i<33554431U; i++){
    arr[i]+=arr[i-1];
  }

  tmp->mpmt_triggers.resize(args->unsorted_data->mpmt_triggers.size());

  bin=0;
  for(unsigned int i=0; i< args->unsorted_data->mpmt_triggers.size(); i++){
    bin=((args->unsorted_data->mpmt_triggers.at(i).GetCoarseCounter() & 4194303U)<<3) | (args->unsorted_data->mpmt_triggers.at(i).GetFineTime() >>13);
    tmp->mpmt_triggers.at(arr[bin]-1)=args->unsorted_data->mpmt_triggers.at(i);
    arr[bin]--; 
  }


  ///////////////////////////////////////////

  
  delete args->unsorted_data;
  args->unsorted_data=0;
  args->data->sorted_data_mtx.lock();
  args->data->sorted_data[tmp->coarse_counter >> 6]=tmp;
  args->data->sorted_data_mtx.unlock();
  
  args->data=0;
  delete args;
  args=0;
  
  return true;
}
