# WLED Matter Light Bridge

An ESPHome custom component that creates a Matter-compatible light controller which forwards all commands to a WLED device via its JSON API.

## Features

- 🌈 Full RGB color control
- 💡 Brightness control
- 🔌 On/Off control
- 🏠 Matter protocol support (via ESPHome)
- 🔗 Integrates with existing WLED installations
- 📡 WiFi connectivity

## Overview

This custom component allows you to control your WLED LED strips through Matter-compatible home automation systems (like Apple Home, Google Home, Amazon Alexa, etc.) by acting as a bridge. When you control the light through Matter, it sends the corresponding JSON API commands to your WLED device.

## Requirements

- ESPHome 2024.11.0 or later (for Matter support)
- ESP32 board (Matter protocol requires ESP32, not ESP8266)
- ESP-IDF framework (required for Matter, not Arduino)
- Existing WLED installation (v0.13.0 or later recommended)
- WiFi network
- Matter-compatible controller (Apple Home, Google Home, Amazon Alexa, etc.)

## Installation

### 1. Clone or Download this Repository

```bash
git clone https://github.com/netmindz/wled-matter.git
cd wled-matter
```

### 2. Create Your Configuration

Copy the example configuration:

```bash
cp example.yaml your-device.yaml
```

Edit `your-device.yaml` and update:
- `wled_host`: IP address or hostname of your WLED device
- `wled_port`: Port of your WLED device (default: 80)
- WiFi credentials (or use secrets.yaml)

### 3. Create Secrets File

```bash
cp secrets.yaml.example secrets.yaml
```

Edit `secrets.yaml` with your WiFi credentials.

### 4. Compile and Upload

Using ESPHome:

```bash
esphome run your-device.yaml
```

Or using ESPHome Dashboard:
1. Add the device configuration to your ESPHome dashboard
2. Click "Install"
3. Choose your preferred installation method (USB, OTA, etc.)

## Configuration

### Basic Configuration

```yaml
esphome:
  name: wled-matter-bridge
  platformio_options:
    board_build.partitions: partitions.csv

esp32:
  board: esp32dev
  framework:
    type: esp-idf  # Matter requires ESP-IDF, not Arduino
    version: recommended
    sdkconfig_options:
      CONFIG_FREERTOS_UNICORE: y
      CONFIG_COMPILER_OPTIMIZATION_SIZE: y
      CONFIG_LWIP_MAX_SOCKETS: "16"

# Matter protocol configuration
esp32_ble:  # Required for Matter commissioning

mdns:
  disabled: false  # Required for Matter discovery

matter:
  vendor_id: 0xFFF1
  product_id: 0x8001
  device_type: dimmable_light

external_components:
  - source:
      type: local
      path: components
    components: [ wled_matter_light ]

light:
  - platform: wled_matter_light
    name: "WLED Light"
    id: wled_light
    wled_host: "192.168.1.100"  # Your WLED device IP
    wled_port: 80                # WLED port (default: 80)
```

### Configuration Options

#### WLED Component Options
| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `wled_host` | string | Yes | - | IP address or hostname of WLED device |
| `wled_port` | int | No | 80 | Port number of WLED HTTP server |
| `name` | string | Yes | - | Name of the light in Home Assistant/Matter |
| `id` | string | Yes | - | Internal ID for the light component |

#### Matter Configuration Options
| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `vendor_id` | hex | Yes | 0xFFF1 | Matter vendor ID (use your own if registered) |
| `product_id` | hex | Yes | 0x8001 | Matter product ID |
| `device_type` | string | Yes | dimmable_light | Matter device type (use dimmable_light or color_temperature_light) |

## How It Works

1. **Matter Protocol Layer**: The ESP32 runs ESPHome with Matter protocol support, creating a Matter endpoint that appears as a dimmable light to Matter controllers
2. **Light Control Interface**: ESPHome's light component receives Matter commands (on/off, brightness, color)
3. **Translation Layer**: The custom component translates Matter light commands into WLED JSON API format
4. **HTTP Communication**: Commands are sent via HTTP POST to the WLED device's JSON API endpoint
5. **WLED Execution**: The WLED device receives the JSON API call and updates the LED strips

**Matter Flow:**
```
Matter Controller (Apple Home/Google/Alexa)
    ↓ (Matter Protocol over Thread/WiFi)
ESP32 with ESPHome + Matter
    ↓ (Light Component)
WLED Matter Light Component
    ↓ (HTTP POST JSON)
WLED Device
    ↓
LED Strips
```

## WLED JSON API

The component uses the following WLED JSON API endpoints and parameters:

- **Endpoint**: `http://<wled_host>:<wled_port>/json/state`
- **Method**: POST
- **Parameters**:
  - `on`: boolean - Turn light on/off
  - `bri`: 0-255 - Brightness level
  - `seg[0].col[0]`: [R,G,B] - RGB color values (0-255)

Example JSON payload:
```json
{
  "on": true,
  "bri": 128,
  "seg": [{
    "col": [[255, 100, 50]]
  }]
}
```

## Troubleshooting

### Light doesn't respond

1. Check that your WLED device is accessible:
   ```bash
   ping <wled_host>
   curl http://<wled_host>/json/state
   ```

2. Check ESPHome logs:
   ```bash
   esphome logs your-device.yaml
   ```

3. Verify network connectivity between ESP32 and WLED device

### Matter pairing fails

1. Ensure you're using ESPHome 2024.5.0 or later
2. Make sure your ESP32 is properly connected to WiFi
3. Check that Matter is properly configured in your ESPHome configuration
4. Try resetting the Matter pairing on your controller

### Compilation errors

1. Ensure you have the latest ESPHome version:
   ```bash
   pip install --upgrade esphome
   ```

2. Check that all required dependencies are installed

3. Make sure you're using ESP-IDF framework (not Arduino) - Matter requires ESP-IDF

## Matter Commissioning

After flashing the firmware to your ESP32:

### Using Apple Home
1. Open the Home app on your iPhone/iPad
2. Tap the "+" button to add an accessory
3. Select "More options..." 
4. Look for "WLED Matter Bridge" (or your configured name)
5. Scan the QR code shown in ESPHome logs or displayed on the device
6. Follow the pairing instructions

### Using Google Home
1. Open the Google Home app
2. Tap "+" then "Set up device"
3. Select "Matter-enabled device"
4. Scan the QR code from ESPHome logs
5. Complete the setup process

### Using Home Assistant
1. Go to Settings → Devices & Services
2. Click "+ Add Integration"
3. Search for "Matter"
4. Follow the commissioning wizard
5. Scan the QR code or enter the pairing code

### Getting the QR Code
The Matter QR code and manual pairing code will be displayed in:
- ESPHome logs during device boot
- ESPHome dashboard (when connected to the device)
- Serial monitor output

**Note**: The device must be connected to WiFi before Matter commissioning can begin.

## Development

### File Structure

```
wled-matter/
├── components/
│   └── wled_matter_light/
│       ├── __init__.py              # Python component definition
│       ├── wled_matter_light.h      # C++ header
│       └── wled_matter_light.cpp    # C++ implementation
├── example.yaml                      # Example ESPHome configuration
├── secrets.yaml.example              # Secrets template
├── .gitignore                        # Git ignore rules
└── README.md                         # This file
```

### Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License - see LICENSE file for details

## Credits

- Built with [ESPHome](https://esphome.io/)
- Compatible with [WLED](https://github.com/Aircoookie/WLED)
- Uses [Matter](https://csa-iot.org/all-solutions/matter/) protocol

## Support

For issues and questions:
- Open an issue on GitHub
- Check ESPHome documentation: https://esphome.io/
- Check WLED documentation: https://kno.wled.ge/
