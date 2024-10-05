#ifndef MPMT_DATA_H
#define MPMT_DATA_H

#include <WCTEMPMTHit.h>
#include <WCTEMPMTLED.h>
#include <WCTEMPMTWaveform.h>
#include <WCTEMPMTPPS.h>

class TriggerInfo;

struct MPMTData{

  unsigned int coarse_counter;
  std::vector<WCTEMPMTHit> mpmt_hits;
  std::vector<WCTEMPMTLED> mpmt_leds;
  std::vector<WCTEMPMTWaveform> mpmt_waveforms;
  std::vector<WCTEMPMTPPS> mpmt_pps;
  std::vector<WCTEMPMTHit> mpmt_triggers;
  unsigned int cumulative_sum[33554431U];
  std::vector<TriggerInfo> unmerged_triggers;
  

};


#endif
