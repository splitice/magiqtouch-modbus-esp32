# ESPHome Setup Guide for MagIQTouch HVAC Control

## ⚠️ Important Notice

This ESPHome configuration is currently a **FRAMEWORK/TEMPLATE** for developers. It provides:
- Complete infrastructure (WiFi, UART, pins, OTA)
- Entity structure and definitions
- Configuration ready for custom component integration

**What's Missing**: A custom C++ component (~1000 lines) to implement the MagIQTouch Modbus protocol.

**For production use**, please use the [Arduino firmware](../ArduinoControlLAN-AirconControl/) with the [Home Assistant integration](https://github.com/mrhteriyaki/magiqtouch-modbus-esp32-ha).

For implementation details, see [ESPHome-Status.md](../ESPHome-Status.md).

---

## Overview

When completed, the ESPHome version will provide native Home Assistant integration with the MagIQTouch HVAC system. The framework includes:

- **Native Home Assistant Integration**: Automatic discovery and configuration
- **Climate Entity**: Full HVAC control through Home Assistant's climate interface
- **Individual Sensors**: Temperature sensors, fan speed, system mode
- **Zone Control**: Separate switches for Zone 1 and Zone 2
- **Drain Mode**: Manual trigger and status monitoring
- **OTA Updates**: Update firmware wirelessly after initial flash
- **Web Interface**: Built-in web server for status monitoring

## Framework Structure

The ESPHome configuration provides the following entity framework:

### Climate Control
- **Climate Entity**: Main HVAC control with temperature, mode, and fan settings

### Sensors
- Zone 1 Temperature
- Zone 2 Temperature
- Fan Speed
- Device Uptime

### Binary Sensors
- Zone 1 Enabled status
- Zone 2 Enabled status
- Drain Mode Active status
- Automatic Clean Running status

### Switches
- Zone 1 Enable/Disable
- Zone 2 Enable/Disable
- Manual Drain Mode Trigger

### Number Controls
- Fan Speed (0-10)

### Text Sensors
- System Mode (External Fan, Recycle Fan, Cooler Manual, Cooler Auto, Heater, Off)

## Prerequisites

### Hardware Requirements
- ESP32 development board (see main [Hardware List](HardwareList.md))
- 2x RS485 modules
- USB cable for initial flashing
- Power supply (5V)

### Software Requirements

**Option 1: Home Assistant (Recommended)**
- Home Assistant OS, Supervised, or Container
- ESPHome add-on installed

**Option 2: Standalone ESPHome**
- Python 3.9 or newer
- pip (Python package manager)
- ESPHome CLI

## Installation Methods

### Method 1: Home Assistant ESPHome Dashboard (Easiest)

1. **Install ESPHome Add-on** (if not already installed)
   - In Home Assistant, go to Settings → Add-ons
   - Click "ADD-ON STORE"
   - Search for "ESPHome"
   - Click "INSTALL"
   - After installation, click "START"
   - Enable "Show in sidebar"

2. **Prepare Configuration Files**
   - Download `magiqtouch-hvac.yaml` from this repository
   - Copy `secrets.yaml.example` to `secrets.yaml`
   - Edit `secrets.yaml` with your credentials:
     ```yaml
     wifi_ssid: "YourWiFiSSID"
     wifi_password: "YourWiFiPassword"
     api_encryption_key: ""  # Will be auto-generated
     ota_password: "choose-a-secure-password"
     fallback_password: "fallback-ap-password"
     ```

3. **Create New Device in ESPHome**
   - Open ESPHome from Home Assistant sidebar
   - Click "+ NEW DEVICE"
   - Click "CONTINUE"
   - Enter device name: `magiqtouch-hvac`
   - Select device type: `ESP32`
   - Click "SKIP INSTALLATION"

4. **Upload Configuration**
   - Click "EDIT" on your new device
   - Replace all content with the contents of `magiqtouch-hvac.yaml`
   - Copy your `secrets.yaml` content into the secrets editor
   - Click "SAVE"

5. **Flash the Device**
   - Connect ESP32 to your computer via USB
   - Click the three dots (⋮) on your device
   - Click "INSTALL"
   - Choose "Plug into this computer"
   - Select the serial port when prompted
   - Wait for compilation and flashing to complete (5-10 minutes first time)

6. **Add to Home Assistant**
   - After successful flash, disconnect and reconnect power to the ESP32
   - Go to Settings → Devices & Services in Home Assistant
   - ESPHome should auto-discover the device
   - Click "CONFIGURE" and enter the encryption key (if prompted)

### Method 2: ESPHome Command Line

1. **Install ESPHome**
   ```bash
   pip3 install esphome
   ```

2. **Prepare Files**
   - Clone this repository or download the necessary files
   - Copy `secrets.yaml.example` to `secrets.yaml`
   - Edit `secrets.yaml` with your credentials

3. **Validate Configuration**
   ```bash
   esphome config magiqtouch-hvac.yaml
   ```

4. **Compile and Flash**
   
   **First-time flash (via USB):**
   ```bash
   esphome run magiqtouch-hvac.yaml
   ```
   
   **OTA update (after initial flash):**
   ```bash
   esphome run magiqtouch-hvac.yaml --device magiqtouch-hvac.local
   # or
   esphome run magiqtouch-hvac.yaml --device 192.168.1.xxx
   ```

5. **Monitor Logs**
   ```bash
   esphome logs magiqtouch-hvac.yaml
   ```

### Method 3: ESPHome Web Flasher (Easiest for Non-Home Assistant Users)

*Note: This method requires a pre-compiled firmware binary*

1. Visit: https://web.esphome.io/
2. Connect your ESP32 via USB
3. Click "CONNECT"
4. Select your device from the list
5. Click "INSTALL" and choose the firmware file
6. Wait for flashing to complete

## Pin Configuration

### Default WiFi Board Pins
```yaml
serial1_rx_pin: "26"  # UART1 RX (Control Panel)
serial1_tx_pin: "27"  # UART1 TX (Control Panel)
serial2_rx_pin: "16"  # UART2 RX (Unit)
serial2_tx_pin: "17"  # UART2 TX (Unit)
rs485_en_pin: "18"    # RS485 Transmit Enable
```

### Ethernet Board Pins
For Ethernet boards (e.g., WT32-ETH01), modify the substitutions in `magiqtouch-hvac.yaml`:

```yaml
substitutions:
  serial1_rx_pin: "5"
  serial1_tx_pin: "17"
  serial2_rx_pin: "14"
  serial2_tx_pin: "15"
  rs485_en_pin: "18"
```

## Troubleshooting

### Device Not Connecting to WiFi

1. Check your `secrets.yaml` credentials
2. Ensure WiFi signal strength is adequate
3. Try connecting to the fallback hotspot:
   - SSID: `magiqtouch-hvac Fallback`
   - Password: From your `secrets.yaml`

### Device Not Appearing in Home Assistant

1. Ensure Home Assistant and ESP32 are on the same network
2. Check ESPHome logs for errors
3. Manually add the device:
   - Settings → Devices & Services → Add Integration
   - Search for "ESPHome"
   - Enter device IP or hostname

### Serial Communication Issues

1. Verify pin connections match your hardware
2. Check RS485 module wiring
3. Enable debug logging in the YAML:
   ```yaml
   logger:
     level: DEBUG
   ```

### OTA Updates Failing

1. Ensure device is online and reachable
2. Try power cycling the device
3. As last resort, re-flash via USB

## Advanced Configuration

### Custom Fan Speeds

Modify the number component range:
```yaml
number:
  - platform: template
    name: "${friendly_name} Fan Speed"
    min_value: 0
    max_value: 10  # Adjust as needed
    step: 1
```

### Adjust Update Intervals

Change sensor update frequency:
```yaml
sensor:
  - platform: template
    update_interval: 10s  # Change from 5s to 10s
```

### Enable Serial Debugging

Uncomment UART debug sections:
```yaml
uart:
  - id: uart_panel
    # ...
    debug:
      direction: BOTH
      dummy_receiver: false
      after:
        timeout: 10ms
```

## Comparison: ESPHome vs Arduino

| Feature | ESPHome | Arduino |
|---------|---------|---------|
| Home Assistant Integration | Native | Requires custom integration |
| OTA Updates | Built-in | Manual implementation |
| Configuration | YAML | C++ code |
| Debugging | Web logs + HA logs | Serial monitor |
| Development Complexity | Low | High |
| REST API | Built-in web_server | Custom implementation |
| Update Process | OTA or dashboard | USB flash required |

## Migration from Arduino Version

If you're migrating from the Arduino version:

1. **Backup your settings**: Note your WiFi credentials and any customizations
2. **Flash ESPHome**: Follow the installation steps above
3. **Update Home Assistant integration**: Remove the old custom integration and use the native ESPHome integration
4. **Verify operation**: Test all functions before permanent installation
5. **No hardware changes needed**: Wiring remains the same

## Support and Resources

- **ESPHome Documentation**: https://esphome.io/
- **Home Assistant ESPHome**: https://www.home-assistant.io/integrations/esphome/
- **Project Issues**: https://github.com/splitice/magiqtouch-modbus-esp32/issues
- **Main README**: [../README.md](../README.md)
- **Hardware List**: [HardwareList.md](HardwareList.md)
- **API Documentation**: [Api.md](Api.md)

## Future Enhancements

Planned features for future ESPHome releases:
- Complete Modbus message handling port from Arduino version
- Enhanced drain mode automation
- Additional diagnostic sensors
- Configuration options for alternative hardware
- Pre-compiled firmware releases for web flasher
