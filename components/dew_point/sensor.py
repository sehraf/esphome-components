import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_EQUATION,
    CONF_HUMIDITY,
    CONF_TEMPERATURE,
    ICON_THERMOMETER,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

dew_point_ns = cg.esphome_ns.namespace("dew_point")
DewPointComponent = dew_point_ns.class_(
    "DewPointComponent", sensor.Sensor, cg.Component
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        icon="mdi:thermometer-water",
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(DewPointComponent),
            cv.Required(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
            cv.Required(CONF_HUMIDITY): cv.use_id(sensor.Sensor),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    temperature_sensor = await cg.get_variable(config[CONF_TEMPERATURE])
    cg.add(var.set_temperature_sensor(temperature_sensor))

    humidity_sensor = await cg.get_variable(config[CONF_HUMIDITY])
    cg.add(var.set_humidity_sensor(humidity_sensor))
