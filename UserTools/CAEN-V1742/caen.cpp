#include <cstring>

#include "caen/caen.hpp"

namespace caen {

namespace comm {

Error::~Error() throw() {
  if (message) delete[] message;
}

const char* Error::what() const throw() {
  if (!message) {
    // CAEN documentation does not specify the maximum length of error string
    char buf[256];
    memset(buf, 0, sizeof(buf));
    CAENComm_DecodeError(code_, buf);
    buf[255] = 0;

    size_t size = strlen(buf) + 1;
    message = new char[size];
    memcpy(message, buf, size);
  };

  return message;
}

#define COMM(function, ...) \
  do { \
    CAENComm_ErrorCode status = CAENComm_ ## function(__VA_ARGS__); \
    if (status != CAENComm_Success) \
      throw Error(status); \
  } while (false)

Device::Device(
    CAENComm_ConnectionType link_type,
    uint32_t arg,
    uint32_t conet,
    uint32_t vme
) {
  COMM(OpenDevice2, link_type, &arg, conet, vme, &handle);
}

Device::Device(CAENComm_ConnectionType link_type, const char* ip) {
  COMM(OpenDevice2, link_type, ip, 0, 0, &handle);
}

Device::Device(CAENComm_ConnectionType link_type, const std::string& ip) {
  COMM(OpenDevice2, link_type, ip.c_str(), 0, 0, &handle);
}

Device::~Device() {
  COMM(CloseDevice, handle);
}

void Device::write32(uint32_t address, uint32_t data) {
  COMM(Write32, handle, address, data);
}

void Device::write16(uint32_t address, uint16_t data) {
  COMM(Write16, handle, address, data);
}

uint32_t Device::read32(uint32_t address) const {
  uint32_t result;
  COMM(Read32, handle, address, &result);
  return result;
}

uint16_t Device::read16(uint32_t address) const {
  uint16_t result;
  COMM(Read16, handle, address, &result);
  return result;
}

} // namespace comm

} // namespace caen
