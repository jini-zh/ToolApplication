#ifndef UNSORTED_DATA_H
#define UNSORTED_DATA_H

struct UnsortedData{
  
std::vector<WCTEMPMTHit> unsorted_mpmt_hits;
std::vector<WCTEMPMTLED> unsorted_mpmt_leds;
std::vector<WCTEMPMTWaveform> unsorted_mpmt_waveforms;
std::vector<WCTEMPMTPPS> unsorted_mpmt_pps;
std::vector<WCTEMPMTHit> unsorted_mpmt_triggers;

};


#endif
