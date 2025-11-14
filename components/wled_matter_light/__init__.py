"""
WLED Matter Light Custom Component for ESPHome

This component creates a Matter-compatible light that forwards commands to a WLED device
via its JSON API.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT_ID,
)

DEPENDENCIES = ["network"]
AUTO_LOAD = ["json"]

CONF_WLED_HOST = "wled_host"
CONF_WLED_PORT = "wled_port"

wled_matter_light_ns = cg.esphome_ns.namespace("wled_matter_light")
WLEDMatterLight = wled_matter_light_ns.class_("WLEDMatterLight", light.LightOutput)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(WLEDMatterLight),
        cv.Required(CONF_WLED_HOST): cv.string,
        cv.Optional(CONF_WLED_PORT, default=80): cv.port,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)

    cg.add(var.set_wled_host(config[CONF_WLED_HOST]))
    cg.add(var.set_wled_port(config[CONF_WLED_PORT]))
