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

- ESPHome 2024.5.0 or later (for Matter support)
- ESP32 board (Matter requires ESP32)
- Existing WLED installation (v0.13.0 or later recommended)
- WiFi network

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

| Option | Type | Required | Default | Description |
|--------|------|----------|---------|-------------|
| `wled_host` | string | Yes | - | IP address or hostname of WLED device |
| `wled_port` | int | No | 80 | Port number of WLED HTTP server |
| `name` | string | Yes | - | Name of the light in Home Assistant/Matter |
| `id` | string | Yes | - | Internal ID for the light component |

## How It Works

1. The component creates a Matter-compatible light endpoint on your ESP32
2. When you control the light via Matter (e.g., from Apple Home):
   - The Matter command is received by ESPHome
   - The component translates it to a WLED JSON API call
   - The JSON request is sent via HTTP POST to your WLED device
3. Your WLED device receives the command and updates the LEDs accordingly

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
