#ifndef VME_READOUT_H
#define VME_READOUT_H

#include <deque>
#include <mutex>
#include <vector>

template <typename Hit>
class VMEReadout {
public:
  template <typename Iterator>
  void push(Iterator begin, Iterator end);
  
  std::deque<std::vector<Hit>> get();
  bool Send(zmq::socket_t* sock){ return true;}
  bool Receive(zmq::socket_t* sock){ return true;}
  unsigned int size(){return readout.size();}
  
private:
  std::mutex mutex;
  std::deque<std::vector<Hit> > readout;
};

template <typename Hit>
template <typename Iterator>
void VMEReadout<Hit>::push(Iterator begin, Iterator end) {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto i = begin; i != end; ++i) readout.push_back(std::move(*i));
};

template <typename Hit>
std::deque<std::vector<Hit>> VMEReadout<Hit>::get() {
  std::lock_guard<std::mutex> lock(mutex);
  return std::move(readout);

  
};
/*
template <typename Hit> bool Send(zmq::socket_t* sock){
  
  bool ret=true;
  
  std::deque<std::vector<Hit> > readout_to_send;
  mutex.lock();
  if(!readout.size()){
    mutex.unlock();
    return false;
  }
  std::swap(readout_to_send, readout); 
  mutex.unlock();
  
  std::vector<zmq::message_t*> messages;
  
  zmq::message_t tmp= new zmq::message_t(4);
  if(sizeof(Hit)==sizeof(QDCHit)) sprintf(tmp.data(), 4, "QDC"); 
  if(sizeof(Hit)==sizeof(TDCHit)) sprintf(tmp.data(), 4, "TDC");
  message.push_back(tmp);    
  
  for(std::deque<std::vector<Hit>>::iterator it=readout.begin(); it!=readout.end(); it++){
    
    zmq::message_t msg = new zmq::message_t(sizeof(Hit) * it->size());
    memcpy(msg.data(), readout.at(i).data(), sizeof(Hit) * it->size());
    messages.push_back(msg);
    
  }
  for(int i=0; i<messages.size()-1; i++){
    ret= ret && sock.send(messages.at(i),ZMQSNDMORE);
    messages.at(i)=0;
  }
  ret= ret && sock.send(messages.at(messages.size()-1));
  
  messages.clear();
  
  return ret;
  
}

bool Receive(zmq::socket_t* sock){

 std::deque<std::vector<Hit> > readout_received;

 std::vector<zmq::message_t*> messages;

 zmq::message_t *tmp = new zmnq::message_t();
 sock.receive(tmp);
   
 while(tmp.more()){
   tmp = new zmnq::message_t();
   sock.receive(tmp);
   messages.push_back(tmp);
}

 if(!messages.size()) return false; // check later

 std::string type(4);
 memcpy(&type, &messages.at(0)->data(), messages.at(0)->length()); //fix me

 if(type=="QDC"){
   for(int i=1; i<messages.size(); i++){
     std::vector<QDCHit> tmp_hit;
     tmphit.resize(messages.at(i).length() / sizeof(QDCHit));
     memcpy(tmp_hit.data(), messages.at(i)->data(), messages.at(i)->length());
     readout_received.push_back(tmp_hit);     
   }
 }
 if(type=="TDC"){
   for(int i=1; i<messages.size(); i++){
     std::vector<TDCHit> tmp_hit;
     tmphit.resize(messages.at(i).length() / sizeof(TDCHit));
     memcpy(tmp_hit.data(), messages.at(i)->data(), messages.at(i)->length());
     readout_received.push_back(tmp_hit);
   }
 }

 mutex.lock();
 readout.insert(readout.end(), readout_received.begin(), readout_received.end());
 mutex.unlock();

 return true;
 
}
*/
#endif
