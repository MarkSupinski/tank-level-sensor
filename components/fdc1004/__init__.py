import esphome.codegen as cg
from esphome.components import i2c
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
fdc1004_ns = cg.esphome_ns.namespace("fdc1004")
FDC1004Component = fdc1004_ns.class_("FDC1004Component", cg.Component, i2c.I2CDevice)
CONF_FDC1004_ID = "fdc1004_id"

CONFIG_SCHEMA = (
    cv.Schema({cv.GenerateID(): cv.declare_id(FDC1004Component)})
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x50))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
