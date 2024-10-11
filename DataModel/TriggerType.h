#ifndef TRIGGER_TYPE_H
#define TRIGGER_TYPE_H

#include <vector>
#include <string>
#include <SerialisableObject.h>
#include <BinaryStream.h>
#include <MPMTData.h>

class DataModel;

enum class TriggerType{ LASER, NHITS, LED, BEAM, MBEAM, EBEAM, NONE, HARD6 };

class TriggerInfo : public SerialisableObject{

public:
  
  TriggerType type;
  unsigned long time;
  std::vector<WCTEMPMTLED> mpmt_LEDs;
  unsigned long spill_num=0;
  unsigned vme_event_num=0; 
  
  bool Print(){
    std::cout<<"Trigger time = "<<time<<std::endl;
    
    return true;
  }
  std::string GetVersion(){return "1.0";}
  bool Serialise(BinaryStream &bs){
    
    bs & type;
    bs & time;
    bs & mpmt_LEDs;

    return true;
  }
  
};

struct Trigger_algo_args:Thread_args{
  Trigger_algo_args(){
    m_data=0;
    sorted_data=0;
    trigger_vars=0;
  };
  ~Trigger_algo_args(){
    m_data=0;
    trigger_vars=0;
    delete sorted_data;
    sorted_data=0;
  };
  MPMTData* sorted_data;
  DataModel* m_data;
  Store* trigger_vars;
};

#endif
