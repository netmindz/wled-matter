#include "wled_matter_light_hub.h"
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"

#ifdef USE_ESP32
#include "mdns.h"
#include "esp_netif.h"
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
#ifdef USE_ESP32
  ESP_LOGD(TAG, "Scanning for WLED devices via mDNS (_wled._tcp)...");

  mdns_result_t *results = nullptr;
  esp_err_t err = mdns_query_ptr("_wled", "_tcp", 3000, 10, &results);

  if (err != ESP_OK) {
    ESP_LOGW(TAG, "mDNS query failed: %s", esp_err_to_name(err));
    return;
  }

  if (results == nullptr) {
    ESP_LOGD(TAG, "No WLED devices found");
    return;
  }

  mdns_result_t *r = results;
  while (r != nullptr) {
    if (r->hostname && r->addr) {
      std::string hostname = r->hostname;

      if (this->discovered_devices_.find(hostname) == this->discovered_devices_.end()) {
        // Extract first IPv4 address
        std::string ip;
        mdns_ip_addr_t *addr = r->addr;
        while (addr != nullptr) {
          if (addr->addr.type == ESP_IPADDR_TYPE_V4) {
            char ip_buf[16];
            esp_ip4addr_ntoa(&addr->addr.u_addr.ip4, ip_buf, sizeof(ip_buf));
            ip = ip_buf;
            break;
          }
          addr = addr->next;
        }

        uint16_t port = r->port > 0 ? r->port : 80;
        std::string name = r->instance_name ? r->instance_name : hostname;

        if (!ip.empty()) {
          ESP_LOGI(TAG, "Discovered WLED device: %s at %s:%d", name.c_str(), ip.c_str(), port);
          this->create_light_for_device_(hostname, ip, port, name);
        }
      }
    }
    r = r->next;
  }

  mdns_query_results_free(results);
#else
  ESP_LOGW(TAG, "mDNS discovery not supported on this platform");
#endif
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
