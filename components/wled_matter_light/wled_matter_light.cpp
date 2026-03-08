#include "wled_matter_light.h"
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"

namespace esphome {
namespace wled_matter_light {

static const char *const TAG = "wled_matter_light";

void WLEDMatterLight::setup() {
  ESP_LOGCONFIG(TAG, "Setting up WLED Matter Light...");
}

void WLEDMatterLight::dump_config() {
  ESP_LOGCONFIG(TAG, "WLED Matter Light:");
  ESP_LOGCONFIG(TAG, "  WLED Host: %s", this->wled_host_.c_str());
  ESP_LOGCONFIG(TAG, "  WLED Port: %d", this->wled_port_);
}

light::LightTraits WLEDMatterLight::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void WLEDMatterLight::write_state(light::LightState *state) {
  if (this->wled_host_.empty())
    return;

  float red, green, blue;
  state->current_values_as_rgb(&red, &green, &blue);

  uint8_t r = static_cast<uint8_t>(red * 255.0f);
  uint8_t g = static_cast<uint8_t>(green * 255.0f);
  uint8_t b = static_cast<uint8_t>(blue * 255.0f);

  float brightness = state->current_values.get_brightness();
  uint8_t bri = static_cast<uint8_t>(brightness * 255.0f);

  bool is_on = state->current_values.is_on();

  ESP_LOGD(TAG, "Setting WLED state: on=%s, bri=%d, rgb=(%d,%d,%d)",
           is_on ? "true" : "false", bri, r, g, b);

  std::string json_payload = "{";
  json_payload += "\"on\":" + std::string(is_on ? "true" : "false");
  json_payload += ",\"bri\":" + to_string(bri);
  json_payload += ",\"seg\":[{\"col\":[[" + to_string(r) + "," + to_string(g) + "," + to_string(b) + "]]}]";
  json_payload += "}";

  this->send_wled_request(json_payload);
}

bool WLEDMatterLight::send_wled_request(const std::string &json_payload) {
  if (!network::is_connected()) {
    ESP_LOGW(TAG, "Not connected to network, cannot send WLED request");
    return false;
  }

  if (this->http_request_ == nullptr) {
    ESP_LOGE(TAG, "HTTP request component not set");
    return false;
  }

  std::string url = "http://" + this->wled_host_ + ":" + to_string(this->wled_port_) + "/json/state";
  ESP_LOGD(TAG, "POST %s  body=%s", url.c_str(), json_payload.c_str());

  std::list<http_request::Header> headers;
  headers.push_back({"Content-Type", "application/json"});

  auto container = this->http_request_->post(url, json_payload, headers);
  if (!container) {
    ESP_LOGW(TAG, "HTTP POST failed");
    return false;
  }

  int status = container->status_code;
  ESP_LOGD(TAG, "HTTP response: %d", status);
  return (status == 200);
}

}  // namespace wled_matter_light
}  // namespace esphome
