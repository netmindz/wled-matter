# Matter Protocol Quick Reference

## What is Matter?

Matter is a unified, open-source connectivity standard for smart home devices. It allows devices from different manufacturers to work together seamlessly across different platforms (Apple Home, Google Home, Amazon Alexa, etc.).

## Why Matter for WLED?

This project creates a Matter bridge that allows you to:
- Control WLED devices through Matter-compatible ecosystems
- Use voice commands via Siri, Google Assistant, or Alexa
- Integrate WLED into scenes and automations in Apple Home, Google Home, etc.
- Avoid cloud dependencies - Matter works locally

## Matter Features in This Project

### Supported Matter Capabilities
- ✅ **On/Off Control** - Turn lights on and off
- ✅ **Brightness Control** - Adjust brightness from 0-100%
- ✅ **RGB Color Control** - Set any RGB color
- ✅ **Local Control** - No cloud required, works on local network
- ✅ **Multi-Platform** - Works with Apple, Google, Amazon ecosystems

### Matter Device Type
This component implements a `dimmable_light` Matter device type, which supports:
- On/Off cluster
- Level Control cluster (brightness)
- Color Control cluster (RGB)

## Technical Details

### Matter Communication Stack
```
Matter Controller (iPhone/Google Home/etc)
    ↓ Matter Protocol (UDP/TCP)
ESP32 (ESPHome with Matter SDK)
    ↓ Light Component API
WLED Matter Light Component
    ↓ HTTP POST with JSON
WLED Device
    ↓
LED Strips
```

### Matter Endpoints
The ESP32 exposes one Matter endpoint:
- **Endpoint 1**: Dimmable Color Light
  - Cluster 0x0006: On/Off
  - Cluster 0x0008: Level Control (Brightness)
  - Cluster 0x0300: Color Control (RGB)

### Network Requirements
- WiFi 2.4GHz (Matter over WiFi)
- Matter controller on the same network
- mDNS enabled for device discovery
- UDP port 5540 (Matter protocol)

### Storage Requirements
Matter requires persistent storage for:
- Fabric information (which controllers are paired)
- Access Control Lists (ACLs)
- Network credentials
- Device configuration

This is why we need a custom partition table with `matter_kvs` partition.

## Commissioning Process

1. **Device Boot**: ESP32 boots and starts Matter stack
2. **Advertising**: Device advertises itself via BLE and mDNS
3. **QR Code**: Matter QR code is generated and shown in logs
4. **Scanning**: User scans QR code with Matter controller app
5. **Pairing**: Secure session is established
6. **Commissioning**: Device joins the Matter fabric
7. **Ready**: Device is now controllable via Matter

## Matter vs Home Assistant API

This component can work with both:

| Feature | Matter | Home Assistant API |
|---------|--------|-------------------|
| Platform Support | Apple, Google, Amazon, etc. | Home Assistant only |
| Cloud Dependency | No (local only) | No (local only) |
| Setup Complexity | Scan QR code | Manual configuration |
| Voice Control | Via platform (Siri, etc.) | Via HA integrations |
| Automation | Platform-native | Home Assistant |

You can enable both simultaneously in the YAML configuration!

## Troubleshooting Matter

### Device won't commission
- Check that BLE is enabled on ESP32
- Verify WiFi is connected
- Ensure controller is on same network
- Check logs for Matter errors
- Try factory reset: flash device with empty NVS

### Device disappears after reboot
- Matter stores fabric info in NVS
- If partition is corrupted, device forgets pairing
- Check partitions.csv is properly configured
- Verify matter_kvs partition is not erased

### Commands not working
- Check if light is properly registered in ESPHome
- Verify WLED device is accessible from ESP32
- Check HTTP logs for WLED communication
- Matter commands should appear in ESPHome logs

## Further Reading

- [Matter Specification](https://csa-iot.org/all-solutions/matter/)
- [ESPHome Matter Documentation](https://esphome.io/components/matter.html)
- [WLED JSON API](https://kno.wled.ge/interfaces/json-api/)
- [ESP-IDF Matter Examples](https://github.com/espressif/esp-matter)
