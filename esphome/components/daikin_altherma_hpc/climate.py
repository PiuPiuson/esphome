import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    modbus,
    sensor,
    number,
    switch,
    binary_sensor,
    button,
    select,
    climate,
)
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_TEMPERATURE,
    CONF_CO2,
    CONF_SPEED_COUNT,
    UNIT_CELSIUS,
    UNIT_PARTS_PER_MILLION,
    UNIT_MINUTE,
    UNIT_HOUR,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_CARBON_DIOXIDE,
    DEVICE_CLASS_SPEED,
    DEVICE_CLASS_OPENING,
    DEVICE_CLASS_SAFETY,
    STATE_CLASS_MEASUREMENT,
    ICON_THERMOMETER,
    ICON_FAN,
    ICON_TIMER,
    ICON_MOLECULE_CO2,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

AUTO_LOAD = [
    "sensor",
    "modbus",
    "switch",
    "number",
    "binary_sensor",
    "button",
    "select",
    "climate",
]

CONF_WATER_TEMPERATURE = "water_temperature"

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
DaikinAlthermaHPCButton = daikin_altherma_hpc_ns.class_(
    "DaikinAlthermaHPCButton", button.Button, cg.Component
)
DaikinAlthermaHPCSelect = daikin_altherma_hpc_ns.class_(
    "DaikinAlthermaHPCSelect", select.Select, cg.Component
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
                icon=ICON_THERMOMETER,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            # -------- BINARY SENSORS ----------
            # cv.Optional(
            #     CONF_BYPASS_OPEN, default={CONF_NAME: "Bypass"}
            # ): binary_sensor.binary_sensor_schema(
            #     device_class=DEVICE_CLASS_OPENING,
            #     entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            # ),
            # ----------- SWITCHES -------------
            # cv.Optional(CONF_HEATER_INSTALLED): switch.switch_schema(
            #     DaikinAlthermaHPCSwitch, entity_category=ENTITY_CATEGORY_CONFIG
            # ).extend(cv.COMPONENT_SCHEMA),
            # ------------ NUMBERS --------------
            # cv.Optional(
            #     CONF_SUPPLY_EXHAUST_RATIO,
            #     default={CONF_NAME: "Supply / Exhaust Ratio"},
            # ): number.number_schema(
            #     DaikinAlthermaHPCNumber,
            #     entity_category=ENTITY_CATEGORY_CONFIG,
            # ).extend(
            #     cv.COMPONENT_SCHEMA
            # ),
            # ------------ BUTTONS --------------
            # cv.Optional(
            #     CONF_RESET_FILTER_HOURS, default={CONF_NAME: "Reset Filter Hours"}
            # ): button.button_schema(
            #     DaikinAlthermaHPCButton, entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            # ).extend(
            #     cv.COMPONENT_SCHEMA
            # ),
            # ------------ SELECTS --------------
            # cv.Optional(
            #     CONF_FILTER_ALARM_INTERVAL, default={CONF_NAME: "Filter Alarm Interval"}
            # ): select.select_schema(
            #     DaikinAlthermaHPCSelect, entity_category=ENTITY_CATEGORY_CONFIG
            # ).extend(
            #     cv.COMPONENT_SCHEMA
            # ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(modbus.modbus_device_schema(0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await modbus.register_modbus_device(var, config)
    await climate.register_climate(var, config)

    # ------------ SENSORS --------------
    if CONF_WATER_TEMPERATURE in config:
        conf = config[CONF_WATER_TEMPERATURE]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_water_temperature_sensor(sens))

    # -------- BINARY SENSORS ----------
    # if CONF_BYPASS_OPEN in config:
    #     conf = config[CONF_BYPASS_OPEN]
    #     sens = await binary_sensor.new_binary_sensor(conf)
    #     cg.add(var.set_bypass_open_sensor(sens))

    # ----------- SWITCHES -------------

    # if CONF_START_AFTER_POWER_LOSS in config:
    #     conf = config[CONF_START_AFTER_POWER_LOSS]
    #     sw = await switch.new_switch(conf)
    #     await cg.register_component(sw, conf)
    #     cg.add(sw.set_parent(var))
    #     cg.add(sw.set_id(CONF_START_AFTER_POWER_LOSS))
    #     cg.add(var.set_power_restore_switch(sw))

    # ------------ NUMBERS --------------

    # if CONF_DEFROST_START_TEMPERATURE in config:
    #     conf = config[CONF_DEFROST_START_TEMPERATURE]
    #     num = await number.new_number(conf, min_value=-9, max_value=5, step=1)
    #     await cg.register_component(num, conf)
    #     cg.add(num.set_parent(var))
    #     cg.add(num.set_id(CONF_DEFROST_START_TEMPERATURE))
    #     cg.add(var.set_defrost_start_temperature_number(num))

    # ------------ BUTTONS --------------
    # if CONF_RESET_FILTER_HOURS in config:
    #     conf = config[CONF_RESET_FILTER_HOURS]
    #     but = await button.new_button(conf)
    #     await cg.register_component(but, conf)
    #     cg.add(but.set_parent(var))
    #     cg.add(but.set_id(CONF_RESET_FILTER_HOURS))

    # ------------ SELECTS --------------
    # if CONF_FILTER_ALARM_INTERVAL in config:
    #     conf = config[CONF_FILTER_ALARM_INTERVAL]
    #     sel = await select.new_select(
    #         conf, options=["45 days", "60 days", "90 days", "180 days"]
    #     )
    #     await cg.register_component(sel, conf)
    #     cg.add(sel.set_parent(var))
    #     cg.add(sel.set_id(CONF_FILTER_ALARM_INTERVAL))
    #     cg.add(var.set_filter_alarm_interval_select(sel))
