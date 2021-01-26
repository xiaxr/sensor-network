#include <stdint.h>

#include <Arduino.h>
#include <EEPROM.h>

#include "device.h"
#include "buffer.h"

namespace xiaxr {
auto device::device_id() -> device_id_t {
  device_id_t id;
  for (int i = 0; i < id.length(); i++) {
    id[i] = EEPROM.read(i + eeprom::device_id_offset);
  }
  return id;
}

auto device::device_name() -> ::String {
  buffer<eeprom::device_name_length + 1> name;
  uint8_t name_length = EEPROM.read(eeprom::device_name_length_offset);
  for (int i = 0; i < name_length; i++) {
    name[i] = EEPROM.read(i + eeprom::device_name_offset);
  }
  name[name_length] = '\0';
  return ::String((char *)name.bytes());
}
} // namespace xiaxr
