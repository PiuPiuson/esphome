import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import (
    CONF_FLOW_CONTROL_PIN,
    CONF_GD0_PIN,
    CONF_GD2_PIN,
)
from esphome import pins

DEPENDENCIES = ["spi"]

cc1101_ns = cg.esphome_ns.namespace("cc1101")
CC1101 = cc1101_ns.class_("CC1101", cg.Component, spi.SPIDevice)
CC1101Device = cc1101_ns.class_("CC1101Device")
MULTI_CONF = True

CONF_CC1101_ID = "cc1101_id"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CC1101),
            cv.Required(CONF_GD0_PIN): pins.gpio_pin_schema,
            cv.Required(CONF_GD2_PIN): pins.gpio_pin_schema,
            cv.Required(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(spi.spi_device_schema)
)
