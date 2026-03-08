# WLED Matter Light Bridge

An ESPHome custom component that creates a Matter-compatible light controller which forwards all commands to a WLED device via its JSON API.

## Features

- 🌈 Full RGB colour control
- 💡 Brightness control
- 🔌 On/Off control
- 🏠 Matter protocol support (via ESPHome)
- 🔗 Integrates with existing WLED installations
- 📡 WiFi connectivity
- 🔍 **Auto-discovery of WLED devices on your network via mDNS**
- 📦 **Support for multiple WLED devices simultaneously**

## Overview

This custom component allows you to control your WLED LED strips through Matter-compatible home automation systems (like Apple Home, Google Home, Amazon Alexa, etc.) by acting as a bridge. When you control the light through Matter, it sends the corresponding JSON API commands to your WLED device.

Auto-discovery mode uses the native esp-idf mDNS API to find all `_wled._tcp` services on your network, automatically creating a Matter light for each device found. Discovered devices are tracked and re-scanned every 60 seconds once the network is connected.

**🔗 See [MATTER.md](MATTER.md) for detailed information about Matter protocol integration.**

## Requirements

- ESPHome 2024.11.0 or later
- ESP32 board (Matter protocol requires ESP32, not ESP8266)
- ESP-IDF framework (required for Matter and native mDNS; **not** Arduino)
- Existing WLED installation (v0.13.0 or later recommended)
- WiFi network
- Matter-compatible controller (Apple Home, Google Home, Amazon Alexa, Home Assistant, etc.)

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/netmindz/wled-matter.git
cd wled-matter
```

### 2. Install ESPHome

Install ESPHome in a Python virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip esphome
```

### 3. Create Your Secrets File

```bash
cp secrets.yaml.example secrets.yaml
```

Edit `secrets.yaml` with your WiFi credentials.

### 4. Compile and Upload

```bash
source .venv/bin/activate
esphome run wled-matter.yaml
```

To compile only (no upload):

```bash
esphome compile wled-matter.yaml
```

## Configuration

The component is structured as two cooperating pieces:

| Part | Purpose |
|------|---------|
| `wled_matter_light` hub | Auto-discovery via mDNS — finds `_wled._tcp` devices every 60 s |
| `light` platform `wled_matter_light` | Manually configured RGB light forwarded to a specific WLED device |

Both can be used together in the same configuration.

### Auto-Discovery + Manual Light (Recommended)

```yaml
esphome:
  name: wled-matter
  friendly_name: wled-matter

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf

logger:

api:
  encryption:
    key: !secret api_key

ota:
  - platform: esphome
    password: !secret ota_password

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  ap:
    ssid: "Wled-Matter Fallback Hotspot"
    password: !secret fallback_password

captive_portal:

esp32_ble:   # Required for Matter commissioning

mdns:
  disabled: false  # Required for Matter AND WLED discovery

# matter:
#   vendor_id: 0xFFF1
#   product_id: 0x8001
#   device_type: dimmable_light

external_components:
  - source:
      type: local
      path: components
    components: [ wled_matter_light ]

# HTTP client used to send commands to WLED devices
http_request:
  id: http_request_id

# Auto-discovery hub — scans for _wled._tcp mDNS services every 60 s
wled_matter_light:
  enable_discovery: true
  discovery_prefix: "WLED"

# Manually configured RGB light forwarded to a specific WLED device
light:
  - platform: wled_matter_light
    name: "WLED Living Room"
    http_request_id: http_request_id
    wled_host: "192.168.1.100"
    wled_port: 80
```

### Configuration Options

#### Auto-Discovery Hub (`wled_matter_light:`)

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_discovery` | boolean | `true` | Enable automatic mDNS discovery of WLED devices |
| `discovery_prefix` | string | `"WLED"` | Prefix added to discovered device names |

#### Manual Light Platform (`light: - platform: wled_matter_light`)

| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `name` | string | Yes | — | Name of the light in Home Assistant / Matter |
| `wled_host` | string | Yes | — | IP address or hostname of the WLED device |
| `wled_port` | int | No | `80` | Port of the WLED HTTP server |
| `http_request_id` | id | Yes | — | ID of the `http_request:` component in your config |

> **Note:** An `http_request:` component must be declared in your YAML and its `id` passed to each `wled_matter_light` light. This is the ESPHome-native HTTP client and works correctly under the esp-idf framework.

## How It Works

1. **Matter Protocol Layer**: The ESP32 runs ESPHome with Matter protocol support, appearing as an RGB light to Matter controllers
2. **Light Control Interface**: ESPHome's light component receives commands (on/off, brightness, colour)
3. **Translation Layer**: The custom component converts light state into WLED JSON API format
4. **HTTP Communication**: Commands are sent via `http_request` (ESPHome native, esp-idf compatible) to the WLED device
5. **mDNS Discovery**: The hub uses the native esp-idf `mdns_query_ptr` API to scan for `_wled._tcp` services every 60 seconds once WiFi is connected

**Flow:**
```
Matter Controller (Apple Home / Google / Alexa / Home Assistant)
    ↓ Matter Protocol
ESP32 running ESPHome
    ↓ light component
wled_matter_light (write_state)
    ↓ HTTP POST /json/state
WLED Device
    ↓
LED Strips
```

## WLED JSON API

- **Endpoint**: `POST http://<wled_host>:<wled_port>/json/state`
- **Payload**:

```json
{
  "on": true,
  "bri": 128,
  "seg": [{ "col": [[255, 100, 50]] }]
}
```

## Troubleshooting

### Light state not being sent on boot

This is expected behaviour — if no `wled_host` is configured yet (e.g. waiting for auto-discovery), `write_state` returns immediately without logging or sending. Commands are only forwarded once a host has been associated with the light.

### Light doesn't respond after discovery

1. Check that the WLED device is reachable:
   ```bash
   ping <wled_host>
   curl http://<wled_host>/json/state
   ```
2. Check ESPHome logs for `wled_matter_light` and `wled_matter_light.hub` entries:
   ```bash
   esphome logs wled-matter.yaml
   ```
3. Ensure `mdns: disabled: false` is set in your YAML

### mDNS discovery finds no devices

- Confirm your WLED firmware is v0.13+ (older versions may not advertise `_wled._tcp`)
- Ensure the ESP32 and WLED devices are on the same network/VLAN
- mDNS does not cross router boundaries by default

### Compilation errors

1. Ensure you are using ESPHome 2024.11.0 or later:
   ```bash
   pip install --upgrade esphome
   ```
2. Confirm `framework: type: esp-idf` is set — Arduino framework is **not** supported
3. Confirm `external_components` uses `type: local, path: components` — not a git URL
4. Confirm `http_request:` is declared in your YAML

### Matter pairing fails

1. Ensure `esp32_ble:` is present in your config (required for commissioning)
2. Ensure `mdns: disabled: false` is set
3. Make sure the device is connected to WiFi before attempting to pair

## Matter Commissioning

After flashing, the Matter QR code and pairing code will be shown in:
- ESPHome logs during boot
- ESPHome dashboard (when connected)
- Serial monitor output

### Apple Home
1. Open Home → **+** → **Add Accessory** → **More options**
2. Scan the QR code from ESPHome logs

### Google Home
1. Open Google Home → **+** → **Set up device** → **Matter-enabled device**
2. Scan the QR code

### Home Assistant
1. Settings → Devices & Services → **+ Add Integration** → **Matter**
2. Follow the commissioning wizard

## File Structure

```
wled-matter/
├── .github/
│   └── copilot-instructions.md      # Copilot guidance (validate with esphome run wled-matter.yaml)
├── components/
│   └── wled_matter_light/
│       ├── __init__.py              # Hub component definition (auto-discovery)
│       ├── light.py                 # Light platform definition (manual RGB lights)
│       ├── wled_matter_light.h      # Light output C++ header
│       ├── wled_matter_light.cpp    # Light output C++ implementation
│       ├── wled_matter_light_hub.h  # Discovery hub C++ header
│       └── wled_matter_light_hub.cpp# Discovery hub C++ implementation (native mDNS)
├── wled-matter.yaml                 # Main ESPHome configuration
├── example.yaml                     # Manual configuration example
├── example-discovery.yaml           # Auto-discovery example
├── partitions.csv                   # Custom partition table
├── secrets.yaml.example             # Secrets template
└── README.md                        # This file
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License - see LICENSE file for details

## Credits

- Built with [ESPHome](https://esphome.io/)
- Compatible with [WLED](https://github.com/Aircoookie/WLED)
- Uses [Matter](https://csa-iot.org/all-solutions/matter/) protocol
- Native mDNS via [esp-idf mdns component](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mdns.html)

## Support

For issues and questions:
- Open an issue on GitHub
- ESPHome docs: https://esphome.io/
- WLED docs: https://kno.wled.ge/
