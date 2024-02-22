import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import modbus
from esphome.const import CONF_ID, CONF_NAME

AUTO_LOAD = ["modbus"]

daikin_hpc_ns = cg.esphome_ns.namespace("daikin_hpc")
DaikinHpcClimate = daikin_hpc_ns.class_("DaikinHpcClimate", cg.PollingComponent, modbus.ModbusDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DaikinHpcClimate),
            cv.Optional(CONF_NAME): cv.string_strict,
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(modbus.modbus_device_schema(0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)
