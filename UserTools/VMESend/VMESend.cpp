#include "VMESend.h"

VMESend_args::VMESend_args():Thread_args(){
  data=0;
  sock=0;
}

VMESend_args::~VMESend_args(){
  data=0;
  delete sock;
  sock=0;

}


VMESend::VMESend():Tool(){}


bool VMESend::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  std::string port="";
  if(!m_variables.Get("port",port)) port="3434";
  std::stringstream connection;
  connection<<"tcp://*:"<<port;
  
  m_util=new Utilities();
  args=new VMESend_args();
  args->data=m_data;
  args->sock=new zmq::socket_t(*m_data->context, ZMQ_DEALER);
  args->sock->bind(connection.str().c_str()); 
  args->items[0].socket=*args->sock;
  args->items[0].fd=0;
  args->items[0].events=ZMQ_POLLOUT;
  args->items[0].revents=0;
  args->items[1].socket=*args->sock;
  args->items[1].fd=0;
  args->items[1].events=ZMQ_POLLIN;
  args->items[1].revents=0;
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool VMESend::Execute(){

  return true;
}


bool VMESend::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void VMESend::Thread(Thread_args* arg){

  VMESend_args* args=reinterpret_cast<VMESend_args*>(arg);

  if(!(args->data->qdc_readout.size()) && !(args->data->tdc_readout.size())){
    usleep(100);
    return;
  }
  
  zmq::poll(&(args->items[0]), 1, 100);
  
  if(args->items[0].revents & ZMQ_POLLOUT){
    
    if(args->data->qdc_readout.size()) args->data->qdc_readout.Send(args->sock);
    if(args->data->tdc_readout.size()) args->data->tdc_readout.Send(args->sock);
    
  }
  
 

}
