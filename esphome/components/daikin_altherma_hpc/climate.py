import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, modbus, number, sensor, switch
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    DEVICE_CLASS_SPEED,
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_FAN,
    ICON_THERMOMETER,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_REVOLUTIONS_PER_MINUTE,
)

AUTO_LOAD = [
    "sensor",
    "modbus",
    "switch",
    "number",
    "climate",
]

CONF_AIR_TEMPERATURE_OFFSET = "air_temperature_offset"
CONF_WATER_TEMPERATURE = "water_temperature"
CONF_LOCK_CONTROLS = "lock_controls"
CONF_FAN_SPEED = "motor_speed"
CONF_MIN_FAN_SPEED_LOW_QUIET = "min_fan_speed_low_night"

ICON_FAN_CHEVRON_DOWN = "mdi:fan-chevron-down"

daikin_altherma_hpc_ns = cg.esphome_ns.namespace("daikin_altherma_hpc")
DaikinAlthermaHPC = daikin_altherma_hpc_ns.class_(
    "DaikinAlthermaHPC", climate.Climate, cg.PollingComponent, modbus.ModbusDevice
)
DaikinAlthermaHPCSwitch = daikin_altherma_hpc_ns.class_(
    "DaikinAlthermaHPCSwitch", switch.Switch, cg.Component
)
DaikinAlthermaHPCNumber = daikin_altherma_hpc_ns.class_(
    "DaikinAlthermaHPCNumber", number.Number, cg.Component
)


CONFIG_SCHEMA = (
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(DaikinAlthermaHPC),
            # ------------ SENSORS --------------
            cv.Optional(
                CONF_WATER_TEMPERATURE,
                default={CONF_NAME: "Water Temperature"},
            ): sensor.sensor_schema(
                accuracy_decimals=1,
                unit_of_measurement=UNIT_CELSIUS,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
                icon="mdi:thermometer-water",
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(
                CONF_FAN_SPEED,
                default={CONF_NAME: "Fan Speed"},
            ): sensor.sensor_schema(
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_SPEED,
                state_class=STATE_CLASS_MEASUREMENT,
                icon=ICON_FAN,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
            ),

            # ----------- SWITCHES -------------
            cv.Optional(
                CONF_LOCK_CONTROLS, default={CONF_NAME: "Lock Controls"}
            ): switch.switch_schema(
                DaikinAlthermaHPCSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon="mdi:lock",
            ).extend(
                cv.COMPONENT_SCHEMA
            ),
            # ------------ NUMBERS --------------
            cv.Optional(
                CONF_AIR_TEMPERATURE_OFFSET,
                default={CONF_NAME: "Air Temperature Offset"},
            ): number.number_schema(
                DaikinAlthermaHPCNumber,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_THERMOMETER,
            ).extend(
                cv.COMPONENT_SCHEMA
            ),
            cv.Optional(
                CONF_MIN_FAN_SPEED_LOW_QUIET,
                default={CONF_NAME: "Minimum Fan Speed in Low and Quiet"},
            ): number.number_schema(
                DaikinAlthermaHPCNumber,
                entity_category=ENTITY_CATEGORY_CONFIG,
                icon=ICON_FAN_CHEVRON_DOWN,
            ).extend(
                cv.COMPONENT_SCHEMA
            ),
        }
    )
    .extend(cv.polling_component_schema("15s"))
    .extend(modbus.modbus_device_schema(0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)
    await climate.register_climate(var, config)

    # ------------ SENSORS --------------
    conf = config[CONF_WATER_TEMPERATURE]
    sens = await sensor.new_sensor(conf)
    cg.add(var.set_water_temperature_sensor(sens))

    conf = config[CONF_FAN_SPEED]
    sens = await sensor.new_sensor(conf)
    cg.add(var.set_fan_speed_sensor(sens))

    # ----------- SWITCHES -------------
    conf = config[CONF_LOCK_CONTROLS]
    sw = await switch.new_switch(conf)
    await cg.register_component(sw, conf)
    cg.add(sw.set_parent(var))
    cg.add(sw.set_id(CONF_LOCK_CONTROLS))
    cg.add(var.set_lock_controls_switch(sw))

    # ------------ NUMBERS --------------
    conf = config[CONF_AIR_TEMPERATURE_OFFSET]
    num = await number.new_number(conf, min_value=-12, max_value=12, step=0.1)
    await cg.register_component(num, conf)
    cg.add(num.set_parent(var))
    cg.add(num.set_id(CONF_AIR_TEMPERATURE_OFFSET))
    cg.add(var.set_air_temperature_offset_number(num))

    conf = config[CONF_MIN_FAN_SPEED_LOW_QUIET]
    num = await number.new_number(conf, min_value=400, max_value=1700, step=1)
    await cg.register_component(num, conf)
    cg.add(num.set_parent(var))
    cg.add(num.set_id(CONF_MIN_FAN_SPEED_LOW_QUIET))
    cg.add(var.set_min_fan_speed_low_night_number(num))