import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import modbus, climate
from esphome.const import CONF_ID

AUTO_LOAD = ["modbus"]

daikin_hpc_ns = cg.esphome_ns.namespace("daikin_hpc")
DaikinBrcClimate = daikin_hpc_ns.class_("DaikinHpcClimate")

CONF_USE_FAHRENHEIT = "use_fahrenheit"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.boolean,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)

    cg.add(var.set_fahrenheit(config[CONF_USE_FAHRENHEIT]))
