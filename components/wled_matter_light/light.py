import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, http_request
from esphome.const import CONF_OUTPUT_ID

from . import wled_matter_light_ns

DEPENDENCIES = ["wled_matter_light", "http_request"]

CONF_WLED_HOST = "wled_host"
CONF_WLED_PORT = "wled_port"
CONF_HTTP_REQUEST_ID = "http_request_id"

WLEDMatterLight = wled_matter_light_ns.class_("WLEDMatterLight", light.LightOutput, cg.Component)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(WLEDMatterLight),
        cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(http_request.HttpRequestComponent),
        cv.Required(CONF_WLED_HOST): cv.string,
        cv.Optional(CONF_WLED_PORT, default=80): cv.port,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)

    http = await cg.get_variable(config[CONF_HTTP_REQUEST_ID])
    cg.add(var.set_http_request(http))
    cg.add(var.set_wled_host(config[CONF_WLED_HOST]))
    cg.add(var.set_wled_port(config[CONF_WLED_PORT]))
