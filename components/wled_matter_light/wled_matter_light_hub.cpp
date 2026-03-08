#include "wled_matter_light_hub.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/network/util.h"
#include "esphome/components/mdns/mdns_component.h"

namespace esphome {
namespace wled_matter_light {

static const char *const TAG = "wled_matter_light.hub";

void WLEDMatterLightHub::setup() {
  ESP_LOGCONFIG(TAG, "Setting up WLED Matter Light Hub...");
  if (this->enable_discovery_) {
    ESP_LOGCONFIG(TAG, "Auto-discovery enabled for WLED devices");
  }
}

void WLEDMatterLightHub::loop() {
  if (!this->enable_discovery_)
    return;
  if (!network::is_connected())
    return;

  uint32_t now = millis();
  if (now - this->last_scan_time_ < this->scan_interval_)
    return;

  this->last_scan_time_ = now;
  this->scan_for_wled_devices_();
}

void WLEDMatterLightHub::dump_config() {
  ESP_LOGCONFIG(TAG, "WLED Matter Light Hub:");
  ESP_LOGCONFIG(TAG, "  Discovery enabled: %s", this->enable_discovery_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Discovery prefix: %s", this->discovery_prefix_.c_str());
  ESP_LOGCONFIG(TAG, "  Scan interval: %d seconds", this->scan_interval_ / 1000);

  if (!this->discovered_devices_.empty()) {
    ESP_LOGCONFIG(TAG, "  Discovered WLED devices:");
    for (const auto &device : this->discovered_devices_) {
      ESP_LOGCONFIG(TAG, "    - %s (%s:%d)", device.second.name.c_str(),
                    device.second.ip_address.c_str(), device.second.port);
    }
  }
}

void WLEDMatterLightHub::scan_for_wled_devices_() {
  // mDNS-based discovery is done at config-time in ESPHome (esp-idf framework).
  // Runtime mDNS queries are not directly supported via ESPHome's mdns component API.
  // Log a notice and skip — devices should be configured statically via the light platform.
  ESP_LOGD(TAG, "Runtime mDNS discovery not available in esp-idf framework; "
                "configure WLED devices statically using the wled_matter_light light platform.");
}

void WLEDMatterLightHub::create_light_for_device_(const std::string &hostname,
                                                   const std::string &ip,
                                                   uint16_t port,
                                                   const std::string &name) {
  ESP_LOGI(TAG, "Tracking device %s at %s:%d", name.c_str(), ip.c_str(), port);

  WLEDDevice device;
  device.hostname = hostname;
  device.ip_address = ip;
  device.port = port;
  device.name = name;
  device.light_output = nullptr;

  this->discovered_devices_[hostname] = device;
}

}  // namespace wled_matter_light
}  // namespace esphome
