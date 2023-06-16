#pragma once

#include "caen.hpp"

#include <CAENDigitizer.h>

namespace caen {

class Digitizer {
  public:
    class Error: public caen::Error {
      public:
        Error(CAEN_DGTZ_ErrorCode code): code_(code) {};

        Error(const char* function, CAEN_DGTZ_ErrorCode code):
          code_(code), function(function)
        {};

        ~Error();

        const char* what() const throw();
        CAEN_DGTZ_ErrorCode code() const { return code_; };

      private:
        CAEN_DGTZ_ErrorCode code_;
        const char* function = nullptr;
        mutable char* message = nullptr;
    };

    class ReadoutBuffer {
      public:
        ReadoutBuffer(ReadoutBuffer&&);
        ReadoutBuffer(const Digitizer&);
        ~ReadoutBuffer();

      private:
        char* memory;
        uint32_t size;

        friend class Digitizer;
    };

    class Event {
      public:
        virtual ~Event() {};
    };

    // Event class for the basic (D-WAVE) firmware
    class WaveEvent : public Event {
      public:
        WaveEvent(WaveEvent&&);
        WaveEvent(const Digitizer&);
        ~WaveEvent();

        CAEN_DGTZ_EventInfo_t info;

        void* data = nullptr;

      private:
        int digitizer;

        WaveEvent() {};

        friend class Digitizer;
    };

    class DPPEvent : public Event {
      public:
        DPPEvent(DPPEvent&&);
        DPPEvent(const Digitizer&);
        ~DPPEvent();
    };

    Digitizer(
        CAEN_DGTZ_ConnectionType link, uint32_t arg, int conet, uint32_t vme
    );
    Digitizer(Digitizer&&);
    ~Digitizer();

    int handle() const { return digitizer; };
    const CAEN_DGTZ_BoardInfo_t& info() const { return info_; };

    uint8_t DPPFirmwareCode(uint8_t channel = 0) const;

    void writeRegister(uint32_t address, uint32_t data);
    uint32_t readRegister(uint32_t address) const;

    void reset();

    void clearData();

    void disableEventAlignedReadout();

    uint32_t getMaxNumEventsBLT() const;
    void     setMaxNumEventsBLT(uint32_t value);

    ReadoutBuffer mallocReadoutBuffer() const;

    void readData(CAEN_DGTZ_ReadMode_t, ReadoutBuffer&) const;

    uint32_t getNumEvents(const ReadoutBuffer&) const;

    Event* allocateEvent() const;
    void getEvent(const ReadoutBuffer&, int32_t number, Event*) const;

    void calibrate();

    uint32_t readTemperature(int32_t channel) const;

    void sendSWTrigger();

    CAEN_DGTZ_TriggerMode_t getSWTriggerMode() const;
    void                    setSWTriggerMode(CAEN_DGTZ_TriggerMode_t);

    CAEN_DGTZ_TriggerMode_t getExtTriggerInputMode() const;
    void                    setExtTriggerInputMode(CAEN_DGTZ_TriggerMode_t);

    CAEN_DGTZ_TriggerMode_t getChannelSelfTrigger(uint32_t channel) const;
    void setChannelSelfTrigger(
        uint32_t channel_mask, CAEN_DGTZ_TriggerMode_t mode
    );

    CAEN_DGTZ_TriggerMode_t getGroupSelfTrigger(uint32_t group) const;
    void setGroupSelfTrigger(uint32_t group_mask, CAEN_DGTZ_TriggerMode_t mode);

    uint32_t getChannelGroupMask(uint32_t group) const;
    void     setChannelGroupMask(uint32_t group, uint32_t channels);

    uint32_t getChannelTriggerThreshold(uint32_t channel) const;
    void setChannelTriggerThreshold(uint32_t channel, uint32_t threshold);

    uint32_t getGroupTriggerThreshold(uint32_t group) const;
    void     setGroupTriggerThreshold(uint32_t group, uint32_t threshold);

    CAEN_DGTZ_RunSyncMode_t getRunSynchronizationMode() const;
    void setRunSynchronizationMode(CAEN_DGTZ_RunSyncMode_t);

    CAEN_DGTZ_IOLevel_t getIOLevel() const;
    void                setIOLevel(CAEN_DGTZ_IOLevel_t);

    CAEN_DGTZ_TriggerPolarity_t getTriggerPolarity(uint32_t channel) const;
    void setTriggerPolarity(uint32_t channel, CAEN_DGTZ_TriggerPolarity_t);

    uint32_t getGroupFastTriggerThreshold(uint32_t group) const;
    void setGroupFastTriggerThreshold(uint32_t group, uint32_t threshold);

    uint32_t getGroupFastTriggerDCOffset(uint32_t group) const;
    void     setGroupFastTriggerDCOffset(uint32_t group, uint32_t offset);

    bool getFastTriggerDigitizing() const;
    void setFastTriggerDigitizing(bool);

    CAEN_DGTZ_TriggerMode_t getFastTriggerMode() const;
    void                    setFastTriggerMode(CAEN_DGTZ_TriggerMode_t);

    CAEN_DGTZ_DRS4Frequency_t getDRS4SamplingFrequency() const;
    void setDRS4SamplingFrequency(CAEN_DGTZ_DRS4Frequency_t);

    CAEN_DGTZ_OutputSignalMode_t getOutputSignalMode() const;
    void setOutputSignalMode(CAEN_DGTZ_OutputSignalMode_t);

    uint32_t getChannelEnableMask() const;
    void     setChannelEnableMask(uint32_t);

    uint32_t getGroupEnableMask() const;
    void     setGroupEnableMask(uint32_t);

    void SWStartAcquisition();
    void SWStopAcquisition();

    uint32_t getRecordLength() const;
    uint32_t getRecordLength(uint32_t channel) const;
    void     setRecordLength(uint32_t size);
    void     setRecordLength(uint32_t channel, uint32_t size);

    uint32_t getPostTriggerSize() const;
    void     setPostTriggerSize(uint32_t percent);

    CAEN_DGTZ_AcqMode_t getAcquisitionMode() const;
    void                setAcquisitionMode(CAEN_DGTZ_AcqMode_t);

    uint32_t getChannelDCOffset(uint32_t channel) const;
    void     setChannelDCOffset(uint32_t channel, uint32_t offset);

    uint32_t getGroupDCOffset(uint32_t group) const;
    void     setGroupDCOffset(uint32_t group, uint32_t offset);

    bool getDESMode() const;
    void setDESMode(bool);

    uint16_t getDecimationFactor() const;
    void     setDecimationFactor(uint16_t);

    CAEN_DGTZ_ZS_Mode_t getZeroSuppressionMode() const;
    void                setZeroSuppressionMode(CAEN_DGTZ_ZS_Mode_t);

    void getChannelZSParams(
        uint32_t channel,
        CAEN_DGTZ_ThresholdWeight_t* weight,
        int32_t* threshold,
        int32_t* nsamp
    ) const;

    void setChannelZSParams(
        uint32_t channel,
        CAEN_DGTZ_ThresholdWeight_t weight,
        int32_t threshold,
        int32_t nsamp
    );

    CAEN_DGTZ_AnalogMonitorOutputMode_t getAnalogMonOutput() const;
    void setAnalogMonOutput(CAEN_DGTZ_AnalogMonitorOutputMode_t);

    void getAnalogInspectionMonParams(
        uint32_t* channelmask,
        uint32_t* offset,
        CAEN_DGTZ_AnalogMonitorMagnify_t*,
        CAEN_DGTZ_AnalogMonitorInspectorInverter_t*
    ) const;

    void setAnalogInspectionMonParams(
        uint32_t channelmask,
        uint32_t offset,
        CAEN_DGTZ_AnalogMonitorMagnify_t,
        CAEN_DGTZ_AnalogMonitorInspectorInverter_t
    );

    bool getEventPackaging() const;
    void setEventPackaging(bool);

    uint32_t getDPPPreTriggerSize(int channel) const;
    void     setDPPPreTriggerSize(int channel, uint32_t samples);

    CAEN_DGTZ_PulsePolarity_t getChannelPulsePolarity(uint32_t channel) const;
    void setChannelPulsePolarity(uint32_t channel, CAEN_DGTZ_PulsePolarity_t);

  private:
    int digitizer;
    CAEN_DGTZ_BoardInfo_t info_;

    Digitizer();
};

#ifndef CAEN_DGTZ_KEEP_MACRO
#undef DGTZ
#endif

}; // namespace caen
