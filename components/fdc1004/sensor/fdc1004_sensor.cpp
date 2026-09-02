#include "fdc1004_sensor.h"
#include "esphome/core/log.h"

namespace esphome::fdc1004 {

static const char *const TAG = "fdc1004.sensor";

void FDC1004Sensor::update() {
  float raw = this->parent_->request_measurement(this->channel_, this->capdac_, RATE_100HZ);
  this->publish_state(raw);
}

void FDC1004Sensor::dump_config() {
  LOG_SENSOR("", "FDC1004 Sensor", this);
  ESP_LOGCONFIG(TAG, "  Channel: %u, CAPDAC: %u", this->channel_ + 1, this->capdac_);
}

}  // namespace esphome::fdc1004
