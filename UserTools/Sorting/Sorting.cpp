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

   args->data->unsorted_data_mtx.lock();

   std::vector<UnsortedData*> data_to_sort;
   std::vector<unsigned long> items_to_remove;

   for(std::map<unsigned long, UnsortedData*>::iterator it= args->data->unsorted_data.begin(); it!= args->data->unsorted_data.end(); it++){
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


  //sort stuff.






  
  
  args->data=0;
  delete args;
  args=0;

  return true;
}
