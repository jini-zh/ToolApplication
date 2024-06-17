#include "Factory.h"
#include "Unity.h"

Tool* Factory(std::string tool){
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="V1290")  ret=new V1290;
if (tool=="V792")   ret=new V792;
if (tool=="Dumper") ret=new Dumper;

return ret;
}

