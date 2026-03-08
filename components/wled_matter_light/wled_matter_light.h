#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/http_request/http_request.h"
#include "esphome/core/log.h"
#include <string>

namespace esphome {
namespace wled_matter_light {

class WLEDMatterLight : public light::LightOutput, public Component {
 public:
  void setup() override;
  void dump_config() override;

  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;

  void set_wled_host(const std::string &host) { this->wled_host_ = host; }
  void set_wled_port(uint16_t port) { this->wled_port_ = port; }
  void set_http_request(http_request::HttpRequestComponent *http) { this->http_request_ = http; }

 protected:
  std::string wled_host_;
  uint16_t wled_port_{80};
  http_request::HttpRequestComponent *http_request_{nullptr};

  bool send_wled_request(const std::string &json_payload);
};

}  // namespace wled_matter_light
}  // namespace esphome
