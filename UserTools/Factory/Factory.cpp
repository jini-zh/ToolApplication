#include "Factory.h"
#include "Unity.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="Configuration") ret=new Configuration;
if (tool=="DummyTool") ret=new DummyTool;
if (tool=="FileWriter") ret=new FileWriter;
if (tool=="LED") ret=new LED;
if (tool=="Monitoring") ret=new Monitoring;
if (tool=="Nhits") ret=new Nhits;
if (tool=="RunControl") ret=new RunControl;
if (tool=="Trigger") ret=new Trigger;
if (tool=="MPMT") ret=new MPMT;
if (tool=="JobManager") ret=new JobManager;
if (tool=="MPMTfakeTrigger") ret=new MPMTfakeTrigger;
if (tool=="V1290") ret=new V1290;
if (tool=="V792") ret=new V792;
if (tool=="V812") ret=new V812;
if (tool=="Dumper") ret=new Dumper;
return ret;
}
