import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import modbus
from esphome.const import CONF_ID

AUTO_LOAD = ["modbus"]

daikin_hpc_ns = cg.esphome_ns.namespace("daikin_hpc")
DaikinHpcClimate = daikin_hpc_ns.class_("DaikinHpcClimate", cg.PollingComponent, modbus.ModbusDevice)

CONF_USE_FAHRENHEIT = "use_fahrenheit"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DaikinHpcClimate),
            cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(modbus.modbus_device_schema(0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)

    cg.add(var.set_fahrenheit(config[CONF_USE_FAHRENHEIT]))
