import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import i2c
from esphome.const import CONF_ID

ac101_ns = cg.esphome_ns.namespace("ac101")
AC101 = ac101_ns.class_("AC101", cg.Component, i2c.I2CDevice)


CONFIG_SCHEMA = (
    cv.Schema({cv.GenerateID(): cv.declare_id(AC101)})
    .extend(i2c.i2c_device_schema(0x1A))
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)