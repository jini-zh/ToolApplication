#ifndef WCTE_MPMT_WAVEFORM_H
#define WCTE_MPMT_WAVEFORM_H

#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <bitset>
#include <string>

#include <BinaryStream.h>
#include <SerialisableObject.h>

using namespace ToolFramework;

class WCTEMPMTWaveformHeader{
  
public:
  
  WCTEMPMTWaveformHeader(){std::fill(std::begin(data), std::end(data), 0);}

  unsigned short GetCardID(){return card_id;}
  unsigned short GetHeader(){return ((data[0] & 0b11000000) >> 6 );}
  unsigned short GetFlags(){return ((data[0] & 0b00111100) >> 2 );}
  unsigned int GetCoarseCounter(){ return ((data[0] & 0b00000011) << 30 ) | (data[1] << 22) |  (data[2] << 14) | (data[3] << 6) | ((data[4] & 0b11111100) >> 2); }
  unsigned short GetChannel(){return  ((data[4] & 0b00000011) << 3) | ((data[5] & 0b11100000) >> 5 );}
  unsigned short GetNumSamples(){return ((data[5] & 0b00011111) << 11 ) | (data[6] << 3) |  ((data[7] & 0b11100000) >> 5);}
  unsigned int GetLength(){ return ((data[7] & 0b00011111) << 12 ) | (data[8] << 4) |  ((data[9] & 0b11110000) >> 4); }
  unsigned short GetReserved(){return (data[9] & 0b00001111);}

  void SetCardID(unsigned short in){ card_id = in;}
  void SetHeader(unsigned short in){ data[0] = (data[0] & 0b00111111) | ((in & 0b00000011) << 6);}
  void SetFlags(unsigned short in){ data[0] = (data[0] & 0b11000011) | ((in & 0b00001111) << 2);}
  void SetCoarseCounter(unsigned int in){
    data[0] = (data[0] & 0b11111100) | (in >> 30);
    data[1] = in >> 22;
    data[2] = in >> 14;
    data[3] = in >> 6;
    data[4] = (data[4] & 0b00000011) | ((in & 0b00111111) << 2);
  }
  void SetChannel(unsigned short in){
    data[4] = (data[4] & 0b11111100) | ((in & 0b00011000) >> 3);
    data[5] = (data[5] & 0b00011111) | ((in & 0b00000111) << 5);
    
  }
  void SetNumSamples(unsigned short in){
    data[5] = (data[5] & 0b11100000) | ((in >> 11) & 0b00011111 );
    data[6] = in >> 3;
    data[7] = (data[7] & 0b00011111) | ((in & 0b00000111) << 5);
  }
  void SetLength(unsigned int in){
    data[7] = (data[7] & 0b11100000) | ((in >> 12) & 0b00011111 );
    data[8] = in >> 4;
    data[9] = (data[9] & 0b00001111) | ((in & 0b00001111) << 4);
  }
  void SetReserved(unsigned short in){data[9] = (data[9] & 0b11110000) | (in & 0b00001111) ; }
  void Print(){
    std::cout<<"header = "<<GetHeader()<<std::endl;
    std::cout<<"flags = "<<GetFlags()<<std::endl;
    std::cout<<"coarse_counter = "<<GetCoarseCounter()<<std::endl;
    std::cout<<"channel = "<<GetChannel()<<std::endl;
    std::cout<<"num_samples = "<<GetNumSamples()<<std::endl;
    std::cout<<"length = "<<GetLength()<<std::endl;
    std::cout<<"reserved = "<<GetReserved()<<std::endl;
    
  }
  
  unsigned char data[10];
  
private:

  unsigned short card_id;
  
};

union UWCTEMPMTWaveformHeader{
  
  UWCTEMPMTWaveformHeader(){bits.reset();}
  WCTEMPMTWaveformHeader waveform;
  std::bitset<80> bits;
  
  
};

class WCTEMPMTWaveform : public SerialisableObject {
  
public:
  
  WCTEMPMTWaveform(){samples.clear();}
  WCTEMPMTWaveform(unsigned short& in_card_id, char* in_data){
    samples.clear();
    header.SetCardID(in_card_id);
    memcpy(&header.data[0], in_data, sizeof(header.data));
    samples.resize(header.GetNumSamples());
    memcpy(samples.data(), (in_data+10), sizeof(samples.data())); //pretty sure this wont work
  }
    
  WCTEMPMTWaveformHeader header;
  std::vector<char> samples;

  bool Print(){

    header.Print();
    return true;
  }
  std::string GetVersion(){return "1.0";}
  
  bool Serialise(BinaryStream &bs){
    
    bs & header;
    bs & samples;

    return true;
  }

  
private:
  
  
  
};

#endif
