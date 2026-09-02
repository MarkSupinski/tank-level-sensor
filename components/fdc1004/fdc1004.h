#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome::fdc1004 {

/// FDC1004 measurement rate (value written into the FDC_CONF register).
enum FDC1004SampleRate {
  RATE_100HZ = 0x01,
  RATE_200HZ = 0x02,
  RATE_400HZ = 0x03,
};

class FDC1004Component : public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;

  /// Read one capacitance measurement. Returns a signed raw value, or NAN on error.
  float request_measurement(uint8_t channel, uint8_t capdac, FDC1004SampleRate rate);

 protected:
  bool write_u16(uint8_t reg, uint16_t value);
  bool read_u16(uint8_t reg, uint16_t *value);
};

}  // namespace esphome::fdc1004
