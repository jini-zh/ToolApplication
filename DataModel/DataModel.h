#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <map>
#include <string>
#include <vector>

#include <zmq.hpp>

//#include <caen++/v1290.hpp>
//#include <caen++/v792.hpp>

//#include "TTree.h"o

#include "Store.h"
#include "BoostStore.h"
//#include "DAQLogging.h"
#include "DAQUtilities.h"
#include "SlowControlCollection.h"
#include "DAQDataModelBase.h"
#include "ThreadLoop.h"
#include "WCTERawData.h"
#include "DAQHeader.h"
#include "WCTEMPMTPPS.h"


using namespace ToolFramework;

/*o*
 * \class DataModel
 *
 * This class Is a transient data model class for your Tools within the ToolChain. If Tools need to comunicate they pass all data objects through the data model. There fore inter tool data objects should be deffined in this class. 
 *
 *
 * $Author: B.Richards $ 
 * $Date: 2019/05/26 18:34:00 $
 * Contact: b.richards@qmul.ac.uk
 *
 */

class DataModel : public DAQDataModelBase {
  
  
public:
  
  DataModel(); ///< Simple constructor 

  //TTree* GetTTree(std::string name);
  //void AddTTree(std::string name,TTree *tree);
  //void DeleteTTree(std::string name,TTree *tree);

  //ThreadLoop vme_readout;
  
  //  std::mutex v1290_mutex;
  //std::list<std::vector<caen::V1290::Packet>> v1290_readout;
  //std::mutex v792_mutex;
  //std::list<std::vector<caen::V792::Packet>> v792_readout;
  
  bool load_config;
  bool change_config;
  bool run_start;
  bool run_stop;
  unsigned long start_time;
  
  JobQueue job_queue;
  unsigned int thread_cap;
  unsigned int thread_num;

  std::mutex unsorted_data_mtx;
  std::map<unsigned int,std::vector<WCTEMPMTHit> > unsorted_mpmt_hits;
  std::map<unsigned int,std::vector<WCTEMPMTLED> > unsorted_mpmt_leds;
  std::map<unsigned int,std::vector<WCTEMPMTWaveform> > unsorted_mpmt_waveforms;
  std::map<unsigned int,std::vector<WCTEMPMTPPS> > unsorted_mpmt_pps;
  std::map<unsigned int,std::vector<WCTEMPMTHit> > unsorted_mpmt_triggers;

  
private:
  
  
  //std::map<std::string,TTree*> m_trees; 
  
  
  
};



#endif
