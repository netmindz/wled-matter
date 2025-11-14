"""
WLED Matter Light Custom Component for ESPHome

This component creates Matter-compatible lights that forward commands to WLED devices
via their JSON API. Supports both manual configuration and auto-discovery.
"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT_ID,
)

# Require network for HTTP communication
DEPENDENCIES = ["network"]
AUTO_LOAD = ["json"]
# Note: Matter integration is handled at the root level in the YAML config

CONF_WLED_HOST = "wled_host"
CONF_WLED_PORT = "wled_port"
CONF_ENABLE_DISCOVERY = "enable_discovery"
CONF_DISCOVERY_PREFIX = "discovery_prefix"

wled_matter_light_ns = cg.esphome_ns.namespace("wled_matter_light")
WLEDMatterLight = wled_matter_light_ns.class_("WLEDMatterLight", light.LightOutput, cg.Component)
WLEDMatterLightHub = wled_matter_light_ns.class_("WLEDMatterLightHub", cg.Component)

# Schema for manual light configuration
LIGHT_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(WLEDMatterLight),
        cv.Required(CONF_WLED_HOST): cv.string,
        cv.Optional(CONF_WLED_PORT, default=80): cv.port,
    }
)

# Schema for discovery hub
HUB_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WLEDMatterLightHub),
    cv.Optional(CONF_ENABLE_DISCOVERY, default=True): cv.boolean,
    cv.Optional(CONF_DISCOVERY_PREFIX, default="WLED"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

# Allow both light platform usage and hub configuration
CONFIG_SCHEMA = cv.Any(
    LIGHT_SCHEMA,
    HUB_SCHEMA,
)


async def to_code(config):
    # Check if this is a hub configuration (has enable_discovery or discovery_prefix)
    if CONF_ENABLE_DISCOVERY in config or CONF_DISCOVERY_PREFIX in config:
        # This is a hub configuration for auto-discovery
        var = cg.new_Pvariable(config[CONF_ID])
        await cg.register_component(var, config)
        
        if CONF_ENABLE_DISCOVERY in config:
            cg.add(var.set_enable_discovery(config[CONF_ENABLE_DISCOVERY]))
        if CONF_DISCOVERY_PREFIX in config:
            cg.add(var.set_discovery_prefix(config[CONF_DISCOVERY_PREFIX]))
    else:
        # This is a manual light configuration
        var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
        await cg.register_component(var, config)
        await light.register_light(var, config)

        cg.add(var.set_wled_host(config[CONF_WLED_HOST]))
        cg.add(var.set_wled_port(config[CONF_WLED_PORT]))
