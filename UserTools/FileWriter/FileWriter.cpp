#include "FileWriter.h"

FileWriter_args::FileWriter_args():Thread_args(){}

FileWriter_args::~FileWriter_args(){}


FileWriter::FileWriter():Tool(){}


bool FileWriter::Initialise(std::string configfile, DataModel &data){

  InitialiseTool(data);
  InitialiseConfiguration(configfile);
  //m_variables.Print();

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities();
  args=new FileWriter_args();
  
  m_util->CreateThread("test", &Thread, args);

  ExportConfiguration();
  
  return true;
}


bool FileWriter::Execute(){

  return true;
}


bool FileWriter::Finalise(){

  m_util->KillThread(args);

  delete args;
  args=0;

  delete m_util;
  m_util=0;

  return true;
}

void FileWriter::Thread(Thread_args* arg){

  FileWriter_args* args=reinterpret_cast<FileWriter_args*>(arg);

}
