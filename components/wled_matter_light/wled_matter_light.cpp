#include "wled_matter_light.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/network/util.h"

#ifdef USE_ESP32
#include <HTTPClient.h>
#endif

#ifdef USE_ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

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
  float red, green, blue;
  state->current_values_as_rgb(&red, &green, &blue);
  
  // Convert float (0-1) to uint8 (0-255)
  uint8_t r = static_cast<uint8_t>(red * 255.0f);
  uint8_t g = static_cast<uint8_t>(green * 255.0f);
  uint8_t b = static_cast<uint8_t>(blue * 255.0f);
  
  // Get brightness
  float brightness = state->current_values.get_brightness();
  uint8_t bri = static_cast<uint8_t>(brightness * 255.0f);
  
  // Get on/off state
  bool is_on = state->current_values.is_on();
  
  ESP_LOGD(TAG, "Setting WLED state: on=%s, bri=%d, rgb=(%d,%d,%d)", 
           is_on ? "true" : "false", bri, r, g, b);
  
  // Build JSON payload
  std::string json_payload = "{";
  json_payload += "\"on\":" + std::string(is_on ? "true" : "false");
  json_payload += ",\"bri\":" + to_string(bri);
  json_payload += ",\"seg\":[{\"col\":[[" + to_string(r) + "," + to_string(g) + "," + to_string(b) + "]]}]";
  json_payload += "}";
  
  // Send request to WLED
  this->send_wled_request(json_payload);
}

bool WLEDMatterLight::send_wled_request(const std::string &json_payload) {
  if (!network::is_connected()) {
    ESP_LOGW(TAG, "Not connected to network, cannot send WLED request");
    return false;
  }

#if defined(USE_ESP32) || defined(USE_ESP8266)
  HTTPClient http;
  
  std::string url = "http://" + this->wled_host_ + ":" + to_string(this->wled_port_) + "/json/state";
  
  ESP_LOGD(TAG, "Sending request to: %s", url.c_str());
  ESP_LOGD(TAG, "Payload: %s", json_payload.c_str());
  
#ifdef USE_ESP32
  http.begin(url.c_str());
#endif

#ifdef USE_ESP8266
  WiFiClient client;
  http.begin(client, url.c_str());
#endif
  
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(json_payload.c_str());
  
  if (httpCode > 0) {
    ESP_LOGD(TAG, "HTTP Response code: %d", httpCode);
    if (httpCode == 200) {
      String response = http.getString();
      ESP_LOGV(TAG, "Response: %s", response.c_str());
      http.end();
      return true;
    }
  } else {
    ESP_LOGW(TAG, "HTTP POST failed, error: %s", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  return false;
#else
  ESP_LOGE(TAG, "HTTP client not supported on this platform");
  return false;
#endif
}

}  // namespace wled_matter_light
}  // namespace esphome
