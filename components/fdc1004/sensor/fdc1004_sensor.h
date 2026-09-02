#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

#include "../fdc1004.h"

namespace esphome::fdc1004 {

class FDC1004Sensor : public sensor::Sensor, public PollingComponent, public Parented<FDC1004Component> {
 public:
  FDC1004Sensor(uint8_t channel, uint8_t capdac) : channel_(channel), capdac_(capdac) {}

  void update() override;
  void dump_config() override;

 protected:
  uint8_t channel_;
  uint8_t capdac_;
};

}  // namespace esphome::fdc1004
