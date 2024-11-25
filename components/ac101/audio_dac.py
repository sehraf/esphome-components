import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.components.audio_dac import AudioDac
from esphome.const import CONF_BITS_PER_SAMPLE, CONF_ID, CONF_SAMPLE_RATE

DEPENDENCIES = ["i2c"]

ac101_ns = cg.esphome_ns.namespace("ac101")
AC101 = ac101_ns.class_("AC101", AudioDac, cg.Component, i2c.I2CDevice)

ac101_resolution = ac101_ns.enum("AC101Resolution")
AC101_BITS_PER_SAMPLE_ENUM = {
    8: ac101_resolution.AC101_RESOLUTION_8_BITS,
    16: ac101_resolution.AC101_RESOLUTION_16_BITS,
    24: ac101_resolution.AC101_RESOLUTION_24_BITS,
    32: ac101_resolution.AC101_RESOLUTION_32_BITS,
}

_validate_bits = cv.float_with_unit("bits", "bit")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AC101),
            cv.Optional(CONF_BITS_PER_SAMPLE, default="16bit"): cv.All(
                _validate_bits, cv.enum(AC101_BITS_PER_SAMPLE_ENUM)
            ),
            # cv.Optional(CONF_MIC_GAIN, default="42DB"): cv.enum(
            #     ES8311_MIC_GAIN_ENUM, upper=True
            # ),
            cv.Optional(CONF_SAMPLE_RATE, default=16000): cv.int_range(min=1),
            # cv.Optional(CONF_USE_MCLK, default=True): cv.boolean,
            # cv.Optional(CONF_USE_MICROPHONE, default=False): cv.boolean,
        }
    )
    .extend(i2c.i2c_device_schema(0x1A))
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_bits_per_sample(config[CONF_BITS_PER_SAMPLE]))
    # cg.add(var.set_mic_gain(config[CONF_MIC_GAIN]))
    cg.add(var.set_sample_frequency(config[CONF_SAMPLE_RATE]))
    # cg.add(var.set_use_mclk(config[CONF_USE_MCLK]))
    # cg.add(var.set_use_mic(config[CONF_USE_MICROPHONE]))
