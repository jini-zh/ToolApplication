#include "VMEReceive.h"

VMEReceive_args::VMEReceive_args():Thread_args(){
  data=0;
  sock=0;
}

VMEReceive_args::~VMEReceive_args(){
  data=0;
  delete sock;
  sock=0;
}


VMEReceive::VMEReceive():Tool(){}


bool VMEReceive::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
  std::string port="";
  if(!m_variables.Get("port",port)) port="3434";
  std::string ip="";
  if(!m_variables.Get("ip",ip)) ip="192.168.10.18";
  std::stringstream connection;
  connection<<"tcp://"<<ip<<":"<<port;
  
  m_util=new Utilities();
  args=new VMEReceive_args();
  args->data=m_data;
  args->sock=new zmq::socket_t(*m_data->context, ZMQ_DEALER);
  args->sock->connect(connection.str().c_str());
  args->items[0].socket=*args->sock;
  args->items[0].fd=0;
  args->items[0].events=ZMQ_POLLIN;
  args->items[0].revents=0;
  args->items[1].socket=*args->sock;
  args->items[1].fd=0;
  args->items[1].events=ZMQ_POLLOUT;
  args->items[1].revents=0;
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool VMEReceive::Execute(){

  return true;
}


bool VMEReceive::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void VMEReceive::Thread(Thread_args* arg){

  VMEReceive_args* args=reinterpret_cast<VMEReceive_args*>(arg);

  zmq::poll(&(args->items[0]), 1, 100);
  
  if(args->items[0].revents & ZMQ_POLLIN){

    zmq::message_t msg_type;
    args->sock->recv(&msg_type);
    
    std::istringstream iss(static_cast<char*>(msg_type.data()));    
    
    if(iss.str()=="QDC") args->data->qdc_readout.Receive(args->sock);
    else if(iss.str()=="TDC") args->data->tdc_readout.Receive(args->sock);
    
  }
  

}
