"""
WLED Matter Light Discovery Component

This component discovers WLED devices on the network via mDNS and creates
Matter-compatible lights for each discovered device.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ["network", "mdns"]
AUTO_LOAD = ["json"]

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
