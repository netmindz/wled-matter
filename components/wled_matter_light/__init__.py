"""
WLED Matter Light Custom Component for ESPHome

This component creates Matter-compatible lights that forward commands to WLED devices
via their JSON API. Supports both manual configuration and auto-discovery.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = ["network"]
AUTO_LOAD = ["json", "light"]

CONF_ENABLE_DISCOVERY = "enable_discovery"
CONF_DISCOVERY_PREFIX = "discovery_prefix"

wled_matter_light_ns = cg.esphome_ns.namespace("wled_matter_light")
WLEDMatterLightHub = wled_matter_light_ns.class_("WLEDMatterLightHub", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WLEDMatterLightHub),
    cv.Optional(CONF_ENABLE_DISCOVERY, default=True): cv.boolean,
    cv.Optional(CONF_DISCOVERY_PREFIX, default="WLED"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_enable_discovery(config[CONF_ENABLE_DISCOVERY]))
    cg.add(var.set_discovery_prefix(config[CONF_DISCOVERY_PREFIX]))
