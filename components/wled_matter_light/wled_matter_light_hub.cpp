#include "wled_matter_light_hub.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/network/util.h"

#ifdef USE_ESP32
#include <ESPmDNS.h>
#endif

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
  if (!this->enable_discovery_) {
    return;
  }
  
  if (!network::is_connected()) {
    return;
  }
  
  uint32_t now = millis();
  if (now - this->last_scan_time_ < this->scan_interval_) {
    return;
  }
  
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
#ifdef USE_ESP32
  ESP_LOGD(TAG, "Scanning for WLED devices via mDNS...");
  
  // Query for _wled._tcp mDNS service
  int n = MDNS.queryService("wled", "tcp");
  
  if (n == 0) {
    ESP_LOGD(TAG, "No WLED devices found");
    return;
  }
  
  ESP_LOGI(TAG, "Found %d WLED device(s)", n);
  
  for (int i = 0; i < n; i++) {
    String hostname = MDNS.hostname(i);
    String ip = MDNS.IP(i).toString();
    uint16_t port = MDNS.port(i);
    
    // Try to get a friendly name from TXT records
    String name = hostname;
    
    // Check if we've already discovered this device
    std::string hostname_str = hostname.c_str();
    if (this->discovered_devices_.find(hostname_str) != this->discovered_devices_.end()) {
      ESP_LOGV(TAG, "Device %s already discovered", hostname_str.c_str());
      continue;
    }
    
    ESP_LOGI(TAG, "New WLED device discovered: %s at %s:%d", 
             name.c_str(), ip.c_str(), port);
    
    // Create a light entity for this device
    this->create_light_for_device_(hostname_str, ip.c_str(), port, name.c_str());
  }
#else
  ESP_LOGW(TAG, "mDNS discovery not supported on this platform");
#endif
}

void WLEDMatterLightHub::create_light_for_device_(const std::string &hostname, 
                                                   const std::string &ip, 
                                                   uint16_t port,
                                                   const std::string &name) {
  ESP_LOGI(TAG, "Creating light entity for %s", name.c_str());
  
  // Create a new WLEDMatterLight instance
  auto *light_output = new WLEDMatterLight();
  light_output->set_wled_host(ip);
  light_output->set_wled_port(port);
  
  // Create a light state
  std::string light_name = this->discovery_prefix_ + " " + name;
  auto *light_state = new light::LightState(light_name.c_str(), light_output);
  
  // Register the light with the app
  App.register_light(light_state);
  App.register_component(light_output);
  
  // Store the device info
  WLEDDevice device;
  device.hostname = hostname;
  device.ip_address = ip;
  device.port = port;
  device.name = name;
  device.light_state = light_state;
  device.light_output = light_output;
  
  this->discovered_devices_[hostname] = device;
  
  ESP_LOGI(TAG, "Successfully created light '%s' for WLED device at %s:%d", 
           light_name.c_str(), ip.c_str(), port);
}

}  // namespace wled_matter_light
}  // namespace esphome
