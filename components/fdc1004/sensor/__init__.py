import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import CONF_CHANNEL, CONF_ID, STATE_CLASS_MEASUREMENT

from .. import CONF_FDC1004_ID, FDC1004Component, fdc1004_ns

DEPENDENCIES = ["fdc1004"]
CONF_CAPDAC = "capdac"

FDC1004Sensor = fdc1004_ns.class_("FDC1004Sensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema(FDC1004Sensor, accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)
    .extend(
        {
            cv.GenerateID(CONF_FDC1004_ID): cv.use_id(FDC1004Component),
            cv.Optional(CONF_CHANNEL, default=1): cv.int_range(min=1, max=4),
            cv.Optional(CONF_CAPDAC, default=0): cv.int_range(min=0, max=31),
        }
    )
    .extend(cv.polling_component_schema("5s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_CHANNEL] - 1, config[CONF_CAPDAC])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_FDC1004_ID])
