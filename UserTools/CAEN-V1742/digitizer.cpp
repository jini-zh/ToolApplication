#include <sstream>

#include "caen/digitizer.hpp"

#define DGTZ(function, ...) \
  do { \
    CAEN_DGTZ_ErrorCode status = CAEN_DGTZ_ ## function(__VA_ARGS__); \
    if (status != CAEN_DGTZ_Success) throw Error(#function, status); \
  } while (0)

#define DEFGETTER0(name, type) \
  type Digitizer::get ## name() const { \
    type result; \
    DGTZ(Get ## name, digitizer, &result); \
    return result; \
  }

#define DEFSETTER0(name, type) \
  void Digitizer::set ## name(type value) { \
    DGTZ(Set ## name, digitizer, value); \
  }

#define DEFPROPERTY0(name, type) \
  DEFGETTER0(name, type); \
  DEFSETTER0(name, type)

#define DEFGETTER1(name, type, type1) \
  type Digitizer::get ## name(type1 arg1) const { \
    type result; \
    DGTZ(Get ## name, digitizer, arg1, &result); \
    return result; \
  }

#define DEFSETTER1(name, type, type1) \
  void Digitizer::set ## name(type1 arg1, type value) { \
    DGTZ(Set ## name, digitizer, arg1, value); \
  }

#define DEFPROPERTY1(name, type, type1) \
  DEFGETTER1(name, type, type1); \
  DEFSETTER1(name, type, type1)

#define DEF_EnaDis_GETTER0(name) \
  bool Digitizer::get ## name() const { \
    CAEN_DGTZ_EnaDis_t mode; \
    DGTZ(Get ## name, digitizer, &mode); \
    return mode != CAEN_DGTZ_DISABLE; \
  }

#define DEF_EnaDis_SETTER0(name) \
  void Digitizer::set ## name(bool value) { \
    DGTZ(Set ## name, digitizer, value ? CAEN_DGTZ_ENABLE : CAEN_DGTZ_DISABLE);\
  }

#define DEF_EnaDis_PROPERTY0(name) \
  DEF_EnaDis_GETTER0(name); \
  DEF_EnaDis_SETTER0(name)

namespace caen {

static const char* strerror(CAEN_DGTZ_ErrorCode error) {
  switch (error) {
    case CAEN_DGTZ_Success:
      return "Operation completed successfully";
    case CAEN_DGTZ_CommError:
      return "Communication error";
    case CAEN_DGTZ_GenericError:
      return "Unspecified error";
    case CAEN_DGTZ_InvalidParam:
      return "Invalid parameter";
    case CAEN_DGTZ_InvalidLinkType:
      return "Invalid link type";
    case CAEN_DGTZ_InvalidHandle:
      return "Invalid device handler";
    case CAEN_DGTZ_MaxDevicesError:
      return "Maximum number of devices exceeded";
    case CAEN_DGTZ_BadBoardType:
      return "Operation not allowed on this type of board";
    case CAEN_DGTZ_BadInterruptLev:
      return "The interrupt level is not allowed";
    case CAEN_DGTZ_BadEventNumber:
      return "The event number if bad";
    case CAEN_DGTZ_ReadDeviceRegisterFail:
      return "Unable to read the registry";
    case CAEN_DGTZ_WriteDeviceRegisterFail:
      return "Unable to write into the registry";
    case CAEN_DGTZ_InvalidChannelNumber:
      return "The channel number is invalid";
    case CAEN_DGTZ_ChannelBusy:
      return "The channel is busy";
    case CAEN_DGTZ_FPIOModeInvalid:
      return "Invalid FPIO mode";
    case CAEN_DGTZ_WrongAcqMode:
      return "Wrong acquisition mode";
    case CAEN_DGTZ_FunctionNotAllowed:
      return "This function is not allowed for this module";
    case CAEN_DGTZ_Timeout:
      return "Communication timeout";
    case CAEN_DGTZ_InvalidBuffer:
      return "The buffer is invalid";
    case CAEN_DGTZ_EventNotFound:
      return "The event is not found";
    case CAEN_DGTZ_InvalidEvent:
      return "The event is invalid";
    case CAEN_DGTZ_OutOfMemory:
      return "Out of memory";
    case CAEN_DGTZ_CalibrationError:
      return "Unable to calibrate the board";
    case CAEN_DGTZ_DigitizerNotFound:
      return "Unable to open the digitizer";
    case CAEN_DGTZ_DigitizerAlreadyOpen:
      return "The digitizer is already open";
    case CAEN_DGTZ_DigitizerNotReady:
      return "The digitizer is not ready to operate";
    case CAEN_DGTZ_InterruptNotConfigured:
      return "The digitizer has not the IRQ configured";
    case CAEN_DGTZ_DigitizerMemoryCorrupted:
      return "The digitizer flash memory is corrupted";
    case CAEN_DGTZ_DPPFirmwareNotSupported:
      return "The digitizer DPP firmware is not supported in this lib version";
    case CAEN_DGTZ_InvalidLicense:
      return "Invalid firware license";
    case CAEN_DGTZ_InvalidDigitizerStatus:
      return "The digitizer is found in a corrupted status";
    case CAEN_DGTZ_UnsupportedTrace:
      return "The given trace is not supported by the digitizer";
    case CAEN_DGTZ_InvalidProbe:
      return "The given probe is not supported for the given digitizer's trace";
    case CAEN_DGTZ_UnsupportedBaseAddress:
      return "The base address is not supported, as in the case of DT and NIM devices";
    case CAEN_DGTZ_NotYetImplemented:
      return "The function is not yet implemented";
    default:
      return "Unknown error";
  };
};

Digitizer::Error::~Error() {
  if (message) delete[] message;
};

const char* Digitizer::Error::what() const throw() {
  if (message) return message;

  const char* err = strerror(code_);
  if (!function) return err;

  int n = snprintf(nullptr, 0, "%s: %s", function, err) + 1;
  message = new char[n];
  snprintf(message, n, "%s: %s", function, err);
  return message;
};

Digitizer::Digitizer(): digitizer(-1) {};

Digitizer::Digitizer(Digitizer&& digitizer):
  digitizer(digitizer.digitizer), // digitizer. digitizer? digitizer! digitizer!
  info_(digitizer.info_)
{
  digitizer.digitizer = -1;
};

Digitizer::Digitizer(
    CAEN_DGTZ_ConnectionType link, uint32_t arg, int conet, uint32_t value
) {
  DGTZ(OpenDigitizer2, link, &arg, conet, value, &digitizer);
  try {
    DGTZ(GetInfo, digitizer, &info_);
  } catch (...) {
    CAEN_DGTZ_CloseDigitizer(digitizer);
    throw;
  };
};

Digitizer::~Digitizer() {
  if (digitizer >= 0) CAEN_DGTZ_CloseDigitizer(digitizer);
};

uint8_t Digitizer::DPPFirmwareCode(uint8_t channel) const {
  if (channel > 0xf) throw Error(CAEN_DGTZ_InvalidChannelNumber);
  uint8_t code = readRegister(0x108c | channel << 8) >> 8 & 0xff;
  if (code & 0x80) return code;
  return STANDARD_FW_CODE;
};

void Digitizer::writeRegister(uint32_t address, uint32_t data) {
  DGTZ(WriteRegister, digitizer, address, data);
};

uint32_t Digitizer::readRegister(uint32_t address) const {
  uint32_t data;
  DGTZ(ReadRegister, digitizer, address, &data);
  return data;
};

void Digitizer::reset() {
  DGTZ(Reset, digitizer);
};

void Digitizer::clearData() {
  DGTZ(ClearData, digitizer);
};

void Digitizer::disableEventAlignedReadout() {
  DGTZ(DisableEventAlignedReadout, digitizer);
};

DEFPROPERTY0(MaxNumEventsBLT, uint32_t);

Digitizer::ReadoutBuffer::ReadoutBuffer(ReadoutBuffer&& buffer) {
  memory = buffer.memory;
  size   = buffer.size;
  buffer.memory = nullptr;
};

Digitizer::ReadoutBuffer::ReadoutBuffer(const Digitizer& digitizer):
  memory(nullptr)
{
  DGTZ(MallocReadoutBuffer, digitizer.handle(), &memory, &size);
};

Digitizer::ReadoutBuffer::~ReadoutBuffer() {
  if (memory) CAEN_DGTZ_FreeReadoutBuffer(&memory);
};

Digitizer::ReadoutBuffer Digitizer::mallocReadoutBuffer() const {
  return ReadoutBuffer(*this);
};

void Digitizer::readData(
    CAEN_DGTZ_ReadMode_t mode, ReadoutBuffer& buffer
) const {
  DGTZ(ReadData, digitizer, mode, buffer.memory, &buffer.size);
};

uint32_t Digitizer::getNumEvents(const ReadoutBuffer& buffer) const {
  uint32_t result;
  DGTZ(GetNumEvents, digitizer, buffer.memory, buffer.size, &result);
  return result;
};

Digitizer::WaveEvent::WaveEvent(WaveEvent&& event):
  data(event.data),
  digitizer(event.digitizer)
{
  event.data = nullptr;
};

Digitizer::WaveEvent::WaveEvent(const Digitizer& digitizer):
  digitizer(digitizer.handle())
{
  DGTZ(AllocateEvent, this->digitizer, &data);
};

Digitizer::WaveEvent::~WaveEvent() {
  if (data) DGTZ(FreeEvent, digitizer, &data);
};

Digitizer::Event* Digitizer::allocateEvent() const {
  uint8_t firmware = DPPFirmwareCode();
  switch (firmware) {
    case STANDARD_FW_CODE:
      return new WaveEvent(*this);
    default:
      std::stringstream ss;
      ss << "caen::Digitizer: unsupported firmware: " << firmware;
      throw std::runtime_error(ss.str());
  };
};

void Digitizer::getEvent(
    const ReadoutBuffer& buffer, int32_t number, Event* event
) const {
  if (auto wave_event = dynamic_cast<WaveEvent*>(event)) {
    char* ptr;
    DGTZ(
        GetEventInfo,
        digitizer,
        buffer.memory,
        buffer.size,
        number,
        &wave_event->info,
        &ptr
    );

    DGTZ(DecodeEvent, digitizer, ptr, &wave_event->data);
    return;
  };

  throw std::runtime_error("caen::Digitizer: unsupported event type");
};

void Digitizer::calibrate() {
  DGTZ(Calibrate, digitizer);
};

uint32_t Digitizer::readTemperature(int32_t channel) const {
  uint32_t result;
  DGTZ(ReadTemperature, digitizer, channel, &result);
  return result;
};

void Digitizer::sendSWTrigger() {
  DGTZ(SendSWtrigger, digitizer);
};

DEFPROPERTY0(SWTriggerMode,       CAEN_DGTZ_TriggerMode_t);
DEFPROPERTY0(ExtTriggerInputMode, CAEN_DGTZ_TriggerMode_t);

CAEN_DGTZ_TriggerMode_t
Digitizer::getChannelSelfTrigger(uint32_t channel) const {
  CAEN_DGTZ_TriggerMode_t result;
  DGTZ(GetChannelSelfTrigger, digitizer, channel, &result);
  return result;
};

void Digitizer::setChannelSelfTrigger(
    uint32_t channels, CAEN_DGTZ_TriggerMode_t mode
) {
  DGTZ(SetChannelSelfTrigger, digitizer, mode, channels);
};

CAEN_DGTZ_TriggerMode_t
Digitizer::getGroupSelfTrigger(uint32_t group) const {
  CAEN_DGTZ_TriggerMode_t result;
  DGTZ(GetGroupSelfTrigger, digitizer, group, &result);
  return result;
};

void Digitizer::setGroupSelfTrigger(
    uint32_t groups, CAEN_DGTZ_TriggerMode_t mode
) {
  DGTZ(SetGroupSelfTrigger, digitizer, mode, groups);
};

DEFPROPERTY1(ChannelGroupMask, uint32_t, uint32_t);
DEFPROPERTY1(ChannelTriggerThreshold, uint32_t, uint32_t);
DEFPROPERTY1(GroupTriggerThreshold, uint32_t, uint32_t);
DEFPROPERTY0(RunSynchronizationMode, CAEN_DGTZ_RunSyncMode_t);
DEFPROPERTY0(IOLevel, CAEN_DGTZ_IOLevel_t);
DEFPROPERTY1(TriggerPolarity, CAEN_DGTZ_TriggerPolarity_t, uint32_t);
DEFPROPERTY1(GroupFastTriggerThreshold, uint32_t, uint32_t);
DEFPROPERTY1(GroupFastTriggerDCOffset, uint32_t, uint32_t);
DEF_EnaDis_PROPERTY0(FastTriggerDigitizing);
DEFPROPERTY0(FastTriggerMode, CAEN_DGTZ_TriggerMode_t);
DEFPROPERTY0(DRS4SamplingFrequency, CAEN_DGTZ_DRS4Frequency_t);
DEFPROPERTY0(OutputSignalMode, CAEN_DGTZ_OutputSignalMode_t);
DEFPROPERTY0(ChannelEnableMask, uint32_t);
DEFPROPERTY0(GroupEnableMask, uint32_t);

void Digitizer::SWStartAcquisition() {
  DGTZ(SWStartAcquisition, digitizer);
};

void Digitizer::SWStopAcquisition() {
  DGTZ(SWStopAcquisition, digitizer);
};

// TODO: check for DPP-PSD or DPP-PHA firmware?
DEFPROPERTY0(RecordLength, uint32_t); 

uint32_t Digitizer::getRecordLength(uint32_t channel) const {
  uint32_t result;
  DGTZ(GetRecordLength, digitizer, &result, channel);
  return result;
};

void Digitizer::setRecordLength(uint32_t channel, uint32_t size) {
  DGTZ(SetRecordLength, digitizer, size, channel);
};

DEFPROPERTY0(PostTriggerSize, uint32_t);
DEFPROPERTY0(AcquisitionMode, CAEN_DGTZ_AcqMode_t);
DEFPROPERTY1(ChannelDCOffset, uint32_t, uint32_t);
DEFPROPERTY1(GroupDCOffset,   uint32_t, uint32_t);
DEF_EnaDis_PROPERTY0(DESMode);
DEFPROPERTY0(DecimationFactor, uint16_t);
DEFPROPERTY0(ZeroSuppressionMode, CAEN_DGTZ_ZS_Mode_t);

void Digitizer::getChannelZSParams(
    uint32_t channel,
    CAEN_DGTZ_ThresholdWeight_t* weight,
    int32_t* threshold,
    int32_t* nsamp
) const {
  DGTZ(GetChannelZSParams, digitizer, channel, weight, threshold, nsamp);
};

void Digitizer::setChannelZSParams(
    uint32_t channel,
    CAEN_DGTZ_ThresholdWeight_t weight,
    int32_t threshold,
    int32_t nsamp
) {
  DGTZ(SetChannelZSParams, digitizer, channel, weight, threshold, nsamp);
};

DEFPROPERTY0(AnalogMonOutput, CAEN_DGTZ_AnalogMonitorOutputMode_t);

void Digitizer::getAnalogInspectionMonParams(
    uint32_t* channels,
    uint32_t* offset,
    CAEN_DGTZ_AnalogMonitorMagnify_t* mf,
    CAEN_DGTZ_AnalogMonitorInspectorInverter_t* ami
) const {
  DGTZ(GetAnalogInspectionMonParams, digitizer, channels, offset, mf, ami);
};

void Digitizer::setAnalogInspectionMonParams(
    uint32_t channels,
    uint32_t offset,
    CAEN_DGTZ_AnalogMonitorMagnify_t mf,
    CAEN_DGTZ_AnalogMonitorInspectorInverter_t ami
) {
  DGTZ(SetAnalogInspectionMonParams, digitizer, channels, offset, mf, ami);
};

DEF_EnaDis_PROPERTY0(EventPackaging);

DEFPROPERTY1(DPPPreTriggerSize, uint32_t, int);
DEFPROPERTY1(ChannelPulsePolarity, CAEN_DGTZ_PulsePolarity_t, uint32_t);

}; // namespace caen
