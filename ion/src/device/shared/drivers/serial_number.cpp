#include "serial_number.h"

#include <config/serial_number.h>
#include <ion.h>

#include "base64.h"

namespace Ion {
namespace Device {
namespace SerialNumber {

void copy(char *buffer) {
  const unsigned char *rawUniqueID =
      reinterpret_cast<const unsigned char *>(Config::UniqueDeviceIDAddress);
  Base64::encode(rawUniqueID, 12, buffer);
  buffer[k_serialNumberLength] = 0;
}

}  // namespace SerialNumber
}  // namespace Device
}  // namespace Ion
