#include "Factory.h"

Tool* Factory(std::string tool){
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="DummyTool") ret=new DummyTool;

  if (tool=="CAEN-V1742") ret=new CAEN_V1742;
return ret;
}

