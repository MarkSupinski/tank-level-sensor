#include "fdc1004.h"
#include "esphome/core/log.h"

namespace esphome::fdc1004 {

static const char *const TAG = "fdc1004";

// FDC1004 register map (16-bit registers, big-endian, MSB first)
static const uint8_t REG_MEAS_MSB[4] = {0x00, 0x02, 0x04, 0x06};
static const uint8_t REG_MEAS_LSB[4] = {0x01, 0x03, 0x05, 0x07};
static const uint8_t REG_CONF_MEAS[4] = {0x08, 0x09, 0x0A, 0x0B};
static const uint8_t REG_FDC_CONF = 0x0C;

// CONF_MEAS register bit layout
static const uint8_t CONF_MEAS_CHA_SHIFT = 13;
static const uint8_t CONF_MEAS_CHB_SHIFT = 10;
static const uint8_t CONF_MEAS_CAPDAC_SHIFT = 5;
static const uint16_t CONF_MEAS_CHB_DISABLED = 0x7;
static const uint16_t CONF_MEAS_CHB_CAPDAC = 0x4;
// FDC_CONF register bit layout
static const uint8_t FDC_CONF_RATE_SHIFT = 10;

bool FDC1004Component::write_u16(uint8_t reg, uint16_t value) {
  // 16-bit registers are written MSB first.
  uint8_t data[2] = {(uint8_t) (value >> 8), (uint8_t) (value & 0xFF)};
  return this->write_bytes(reg, data, 2);
}

bool FDC1004Component::read_u16(uint8_t reg, uint16_t *value) {
  uint8_t data[2];
  if (!this->read_bytes(reg, data, 2))
    return false;
  *value = ((uint16_t) data[0] << 8) | data[1];
  return true;
}

void FDC1004Component::setup() {
  // Probe the device (the DEVICE_ID/probe register). A successful read means it is present.
  uint16_t probe;
  if (this->read_u16(0xFF, &probe))
    ESP_LOGI(TAG, "FDC1004 found on I2C");
  else
    ESP_LOGW(TAG, "FDC1004 not responding on I2C");
}

void FDC1004Component::dump_config() { LOG_I2C_DEVICE(this); }

float FDC1004Component::request_measurement(uint8_t channel, uint8_t capdac, FDC1004SampleRate rate) {
  // Use measurement slot == channel (channel is 0-based)
  uint8_t meas = channel;

  // 1) Configure the measurement register for this channel.
  uint16_t conf = ((uint16_t) channel) << CONF_MEAS_CHA_SHIFT;  // CHA = channel
  if (capdac > 0)
    conf |= (CONF_MEAS_CHB_CAPDAC << CONF_MEAS_CHB_SHIFT);  // CHB in CAPDAC mode
  else
    conf |= (CONF_MEAS_CHB_DISABLED << CONF_MEAS_CHB_SHIFT);  // CHB disabled (single-ended)
  conf |= ((uint16_t) capdac) << CONF_MEAS_CAPDAC_SHIFT;

  if (!this->write_u16(REG_CONF_MEAS[meas], conf))
    return NAN;

  // 2) Trigger a single measurement.
  uint16_t trig = ((uint16_t) rate) << FDC_CONF_RATE_SHIFT;
  trig |= (uint16_t) (1 << (7 - meas));  // enable measurement (no repeat)
  if (!this->write_u16(REG_FDC_CONF, trig))
    return NAN;

  // 3) Wait for the measurement-done flag (bit (3 - meas) in FDC_CONF).
  for (uint8_t i = 0; i < 50; i++) {
    uint16_t fdc_conf;
    if (!this->read_u16(REG_FDC_CONF, &fdc_conf))
      return NAN;
    if (fdc_conf & (1 << (3 - meas)))
      break;
    delay(2);
  }

  // 4) Read the result (signed 16-bit upper word, as in the reference driver).
  uint16_t msb, lsb;
  if (!this->read_u16(REG_MEAS_MSB[meas], &msb))
    return NAN;
  if (!this->read_u16(REG_MEAS_LSB[meas], &lsb))
    return NAN;

  return (float) (int16_t) msb;
}

}  // namespace esphome::fdc1004
