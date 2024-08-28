#include "MPMT.h"

MPMT_args::MPMT_args():Thread_args(){}

MPMT_args::~MPMT_args(){}


MPMT::MPMT():Tool(){}


bool MPMT::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();
  
  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();

  m_threadnum=0;
  CreateThread();
  
  m_freethreads=1;
  
  ExportConfiguration();    
  
  return true;
}


bool MPMT::Execute(){
  /*
  for(unsigned int i=0; i<args.size(); i++){
    if(args.at(i)->busy==0){
      *m_log<<"reply="<<args.at(i)->message<<std::endl;
      args.at(i)->message="Hi";
      args.at(i)->busy=1;
      break;
    }

  }

  m_freethreads=0;
  unsigned int lastfree=0;
  for(unsigned int i=0; i<args.size(); i++){
    if(args.at(i)->busy==0){
      m_freethreads++;
      lastfree=i; 
    }
  }

  if(m_freethreads<1) CreateThread();
  if(m_freethreads>1) DeleteThread(lastfree);
  
  *m_log<<ML(1)<<"free threads="<<m_freethreads<<":"<<args.size()<<std::endl;
  MLC();
  
  // sleep(1);  for single tool testing
  */
  return true;
}


bool MPMT::Finalise(){

  for(unsigned int i=0;i<args.size();i++) DeleteThread(0);
  
  args.clear();
  
  delete m_util;
  m_util=0;

  return true;
}

void MPMT::CreateThread(){

  MPMT_args* tmparg=new MPMT_args();
 
  tmparg->data_sock=new zmq::socket_t(*(m_data->context), ZMQ_ROUTER);
  
  tmparg->utils= new DAQUtilities(m_data->context);
  
  tmparg->items[0].socket=*tmparg->data_sock;
  tmparg->items[0].fd=0;
  tmparg->items[0].events=ZMQ_POLLIN;
  tmparg->items[0].revents=0;
  
  tmparg->period=boost::posix_time::seconds(10);
  
  tmparg->last=boost::posix_time::microsec_clock::universal_time();
  tmparg->m_data=m_data;
  
  args.push_back(tmparg);
  std::stringstream tmp;
  tmp<<"T"<<m_threadnum;
  m_util->CreateThread(tmp.str(), &Thread, args.at(args.size()-1));
  m_threadnum++;

}

 void MPMT::DeleteThread(unsigned int pos){

   m_util->KillThread(args.at(pos));

   delete args.at(pos)->data_sock;
   args.at(pos)->data_sock=0;

   delete args.at(pos)->utils;
   args.at(pos)->utils=0;
   
   delete args.at(pos);
   args.at(pos)=0;
   args.erase(args.begin()+(pos));
   

 }

void MPMT::Thread(Thread_args* arg){

  MPMT_args* args=reinterpret_cast<MPMT_args*>(arg);

  args->lapse = args->period -( boost::posix_time::microsec_clock::universal_time() - args->last);
  
  if( args->lapse.is_negative()){
    unsigned short num_connections = args->connections.size();
    if(args->utils->UpdateConnections("MPMT", args->data_sock, args->connections, args->data_port) > num_connections) args->m_data->services->SendLog("Info: New MPMT connected",4);
    
    args->last= boost::posix_time::microsec_clock::universal_time();
  }
  
  zmq::poll(&(args->items[0]), 1, 100);
  
  if(args->items[0].revents & ZMQ_POLLIN){
    
    zmq::message_t identity;
    zmq::message_t* daq_header = new zmq::message_t;
    zmq::message_t* mpmt_data = new zmq::message_t;

    args->message_size=0;
    args->no_data=false;
    
    args->message_size=args->data_sock->recv(&identity);     

    if(!identity.more() || args->message_size == 0){
      args->m_data->services->SendLog("Warning: MPMT thread identity has no size or only message",3);
      delete mpmt_data;
      mpmt_data=0;
      delete daq_header;
      daq_header=0;
      return;
    }
    
    args->message_size=args->data_sock->recv(daq_header);
    
    if(args->message_size == 0){
      args->m_data->services->SendLog("Warning: MPMT thread daq header has no size",3);
      delete mpmt_data;
      mpmt_data=0;
      delete daq_header;
      daq_header=0;
      return;
    }
    if(!daq_header->more()) args->no_data=true;
    else{
      args->message_size=args->data_sock->recv(mpmt_data);
      if(!mpmt_data->more() || args->message_size == 0){
	args->m_data->services->SendLog("ERROR: MPMT thread too many message parts or no data, throwing away data",2);
	zmq::message_t throwaway;
	 args->message_size=args->data_sock->recv(&throwaway);
	 while(throwaway.more()) args->message_size=args->data_sock->recv(&throwaway);
	 delete mpmt_data;
	 mpmt_data=0;
	 delete daq_header;
	 daq_header=0;
	 return;
      }
    }
    
    zmq::message_t reply(sizeof(reply));

    memcpy(reply.data(), daq_header->data(), sizeof(reply));
         
    args->data_sock->send(identity, ZMQ_SNDMORE);
    args->data_sock->send(reply);

    if(args->no_data){
      delete mpmt_data;
      mpmt_data=0;
      delete daq_header;
      daq_header=0;
    }
    else{
      Job* tmp_job= new Job("MPMT");
      MPMTMessages* tmp_msgs= new MPMTMessages;
      tmp_msgs->daq_header=daq_header;
      tmp_msgs->mpmt_data=mpmt_data;
      tmp_msgs->m_data=args->m_data;
      tmp_job->data= tmp_msgs;
      tmp_job->func=ProcessData;
      args->job_queue->AddJob(tmp_job);
    }
  }
  
}



bool MPMT::ProcessData(void* data){
  
  MPMTMessages* msgs=reinterpret_cast<MPMTMessages*>(data);
  
  DAQHeader* daq_header=reinterpret_cast<DAQHeader*>(msgs->daq_header->data());
  unsigned int bin= daq_header->GetCoarseCounter() >> 4; //might not be worth rounding
  unsigned short card_id = daq_header->GetCardID();
  unsigned short card_type = daq_header->GetCardType();
  unsigned long bits=msgs->mpmt_data->size()*8;
  unsigned long current_bit=0;
  
  std::vector<WCTEMPMTHit> vec_mpmt_hit;
  std::vector<WCTEMPMTLED> vec_mpmt_led;
  std::vector<WCTEMPMTPPS> vec_mpmt_pps;
  std::vector<WCTEMPMTWaveform> vec_mpmt_waveform;
  
  char* mpmt_data= reinterpret_cast<char*>(msgs->mpmt_data->data());

  while(current_bit<bits)
    
    if((mpmt_data[current_bit] >> 6) == 1U){ //its a hit or led or pps
      
      if(((mpmt_data[current_bit] >> 2) & 0b00001111 ) == 0U){ // its normal mpmt hit
	WCTEMPMTHit tmp(card_id, &mpmt_data[current_bit]);
	current_bit+=11;
	vec_mpmt_hit.push_back(tmp);
      }
      
      else if(((mpmt_data[current_bit] >> 2) & 0b00001111 ) == 1U){;}// its a pedistal (dont know) 
      
      else if(((mpmt_data[current_bit] >> 2) & 0b00001111 ) == 2U){ // its LED
	WCTEMPMTLED tmp(card_id, &mpmt_data[current_bit]);
	current_bit+=9;
	vec_mpmt_led.push_back(tmp);
      }

      else if(((mpmt_data[current_bit] >> 2) & 0b00001111 ) == 3U){;}// its calib??

      else if(((mpmt_data[current_bit] >> 2) & 0b00001111 ) == 15U){// its PPS
	  WCTEMPMTPPS tmp(card_id, &mpmt_data[current_bit]);
	  current_bit+=17;
	  vec_mpmt_pps.push_back(tmp);
      }
    }
    else if ((mpmt_data[current_bit] >> 6) == 2U){ //its a waveform
        WCTEMPMTWaveform tmp(card_id, &mpmt_data[current_bit]);
	current_bit+= 10 + (tmp.header.GetNumSamples() * 12);
	vec_mpmt_waveform.push_back(tmp);
    }


  msgs->m_data->unsorted_data_mtx.lock();
  if(card_type<2U){ //WCTEMPMT and buffered ADC
    msgs->m_data->unsorted_mpmt_hits[bin].insert( msgs->m_data->unsorted_mpmt_hits[bin].end(), vec_mpmt_hit.begin(), vec_mpmt_hit.end());
    msgs->m_data->unsorted_mpmt_leds[bin].insert( msgs->m_data->unsorted_mpmt_leds[bin].end(), vec_mpmt_led.begin(), vec_mpmt_led.end());
    msgs->m_data->unsorted_mpmt_pps[bin].insert( msgs->m_data->unsorted_mpmt_pps[bin].end(), vec_mpmt_pps.begin(), vec_mpmt_pps.end());
    msgs->m_data->unsorted_mpmt_waveforms[bin].insert( msgs->m_data->unsorted_mpmt_waveforms[bin].end(), vec_mpmt_waveform.begin(), vec_mpmt_waveform.end());
  }
  
  else if(card_type==3U){ //trigger card
    msgs->m_data->unsorted_mpmt_triggers[bin].insert( msgs->m_data->unsorted_mpmt_triggers[bin].end(), vec_mpmt_hit.begin(), vec_mpmt_hit.end());
    msgs->m_data->unsorted_mpmt_pps[bin].insert( msgs->m_data->unsorted_mpmt_pps[bin].end(), vec_mpmt_pps.begin(), vec_mpmt_pps.end());
  }
  msgs->m_data->unsorted_data_mtx.unlock();
  
  delete msgs;
  msgs=0;
  
  return true;
  
}
