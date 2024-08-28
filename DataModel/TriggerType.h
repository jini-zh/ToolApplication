#ifndef TRIGGER_TYPE_H
#define TRIGGER_TYPE_H

#include <vector>
#include <string>
#include <SerialisableObject.h>
#include <BinaryStream.h>


enum class TriggerType{ LASER, NHITS, LED, BEAM, NONE };

class TriggerInfo : public SerialisableObject{

public:
  
  TriggerType type;
  unsigned long time;
  std::vector<WCTEMPMTLED> mpmt_LEDs;
  
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

#endif
