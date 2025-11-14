#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/light/light_state.h"
#include "wled_matter_light.h"
#include <vector>
#include <string>
#include <map>

#ifdef USE_ESP32
#include <ESPmDNS.h>
#endif

namespace esphome {
namespace wled_matter_light {

struct WLEDDevice {
  std::string hostname;
  std::string ip_address;
  uint16_t port;
  std::string name;
  light::LightState *light_state;
  WLEDMatterLight *light_output;
};

class WLEDMatterLightHub : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  
  void set_enable_discovery(bool enable) { this->enable_discovery_ = enable; }
  void set_discovery_prefix(const std::string &prefix) { this->discovery_prefix_ = prefix; }

 protected:
  bool enable_discovery_{true};
  std::string discovery_prefix_{"WLED"};
  uint32_t last_scan_time_{0};
  uint32_t scan_interval_{60000};  // Scan every 60 seconds
  
  std::map<std::string, WLEDDevice> discovered_devices_;
  
  void scan_for_wled_devices_();
  void create_light_for_device_(const std::string &hostname, const std::string &ip, uint16_t port, const std::string &name);
};

}  // namespace wled_matter_light
}  // namespace esphome
