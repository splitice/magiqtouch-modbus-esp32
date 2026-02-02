# MagIQTouch Modbus

This project is an alternative to the official WiFi Module for the MagiqTouch system.  
It provides local access and control with a simple [REST API](Docs/Api.md).

## 🚀 Quick Start

### Arduino Firmware (Production-Ready - Recommended)

The Arduino firmware is **fully functional and tested** with all features working:
- Complete Modbus protocol implementation
- REST API for control
- Drain mode automation
- Zone control
- Web interface

**See [Arduino IDE Setup](#arduino-ide-setup-production-ready) below for installation instructions.**

### ESPHome Component (Production-Ready)

A complete ESPHome component is provided with full Modbus protocol implementation and native Home Assistant integration.

✅ **Production Ready**: Complete C++ component with all features implemented.

See the complete example configuration below for setup instructions.

---

## 📋 ESPHome Component Features

**Status**: Production-ready with full Modbus protocol implementation.

### What's Included
- Complete C++ Modbus protocol implementation
- Single UART configuration for RS485
- Native ESPHome button controls for all functions
- Temperature sensor support
- Drain mode automation
- WiFi, API, and OTA support
- Debug logging at multiple levels

### Control Interface
- Power ON/OFF buttons
- 5 mode selection buttons (Fan External, Fan Recycle, Cooler Manual, Cooler Auto, Heater)
- Fan speed slider (0-10)
- Drain mode trigger/cancel buttons
- Thermistor temperature sensor
- System mode status display

---

## 📝 Complete Example Configuration

Here's a **full, copy-paste ready** example configuration for using the MagIQTouch ESPHome component.

### 1. Using as External Component (From GitHub)

Create a file named `magiqtouch-hvac.yaml` with the following content:

```yaml
substitutions:
  device_name: magiqtouch-hvac
  friendly_name: MagIQTouch HVAC Control
  
  # Pin Configuration for WiFi Boards (ESP32 Dev Module, FireBeetle, etc.)
  uart_rx_pin: "26"      # UART RX - connects to RS485 RO (Receiver Output)
  uart_tx_pin: "27"      # UART TX - connects to RS485 DI (Driver Input)
  rs485_en_pin: "18"     # RS485 DE/RE - Driver/Receiver Enable (tie together)
  
  # Pin Configuration for Ethernet Boards (uncomment and adjust if using ethernet)
  # uart_rx_pin: "5"
  # uart_tx_pin: "17"
  # rs485_en_pin: "14"

esphome:
  name: ${device_name}
  friendly_name: ${friendly_name}
  comment: "MagIQTouch Modbus HVAC Controller"

# Use the component from this GitHub repository
external_components:
  - source:
      type: git
      url: https://github.com/splitice/magiqtouch-modbus-esp32
      ref: main  # or specify a branch/tag
    components: [magiqtouch]
    refresh: 0s  # Use "always" during development to always fetch latest

esp32:
  board: esp32dev  # Change to your board type (firebeetle, esp32-s3-devkitc-1, etc.)
  framework:
    type: arduino

# Enable logging (disable UART logging to avoid conflicts with RS485)
logger:
  level: INFO
  baud_rate: 0

# Enable Home Assistant API
api:
  encryption:
    key: !secret api_encryption_key

# Enable OTA updates
ota:
  - platform: esphome
    password: !secret ota_password

# WiFi Configuration
wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password
  power_save_mode: none
  
  # Fallback hotspot in case WiFi fails
  ap:
    ssid: "${device_name} Fallback"
    password: !secret fallback_password

captive_portal:

# Optional: Web server for monitoring
web_server:
  port: 80
  version: 2

# UART Configuration for RS485 Modbus
uart:
  - id: uart_modbus
    tx_pin: GPIO${uart_tx_pin}
    rx_pin: GPIO${uart_rx_pin}
    baud_rate: 9600
    data_bits: 8
    parity: NONE
    stop_bits: 1

# MagiqTouch Component
magiqtouch:
  - id: magiqtouch_hvac
    uart_id: uart_modbus
    rs485_enable_pin: GPIO${rs485_en_pin}
    drain_mode_active_sensor: drain_mode_active_sensor
    system_mode_sensor: system_mode_sensor

# Binary Sensors
binary_sensor:
  - platform: template
    name: "${friendly_name} Drain Mode Active"
    id: drain_mode_active_sensor
    icon: "mdi:water-pump"

# System Mode Text Sensor
text_sensor:
  - platform: template
    name: "${friendly_name} System Mode"
    id: system_mode_sensor
    icon: "mdi:air-conditioner"

# Control Buttons
button:
  - platform: template
    name: "${friendly_name} Power ON"
    icon: "mdi:power"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_power(true);
  
  - platform: template
    name: "${friendly_name} Power OFF"
    icon: "mdi:power-off"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_power(false);
  
  - platform: template
    name: "${friendly_name} Mode: Fan External"
    icon: "mdi:fan"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_mode(0);
  
  - platform: template
    name: "${friendly_name} Mode: Fan Recycle"
    icon: "mdi:fan"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_mode(1);
  
  - platform: template
    name: "${friendly_name} Mode: Cooler Manual"
    icon: "mdi:snowflake"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_mode(2);
  
  - platform: template
    name: "${friendly_name} Mode: Cooler Auto"
    icon: "mdi:snowflake-thermometer"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_mode(3);
  
  - platform: template
    name: "${friendly_name} Mode: Heater"
    icon: "mdi:fire"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_mode(4);
  
  - platform: template
    name: "${friendly_name} Trigger Drain Mode"
    icon: "mdi:water-pump"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->trigger_drain_mode();
  
  - platform: template
    name: "${friendly_name} Cancel Drain Mode"
    icon: "mdi:water-pump-off"
    on_press:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->cancel_drain_mode();

# Fan Speed Control
number:
  - platform: template
    name: "${friendly_name} Fan Speed"
    id: fan_speed_number
    icon: "mdi:fan"
    min_value: 0
    max_value: 10
    step: 1
    mode: slider
    optimistic: true
    set_action:
      - lambda: |-
          auto controller = (esphome::magiqtouch::MagiqTouchComponent*)id(magiqtouch_hvac);
          controller->set_fan_speed((uint8_t)x);
```

### 2. Secrets File (secrets.yaml)

Create a file named `secrets.yaml` in the same directory:

```yaml
# WiFi Credentials
wifi_ssid: "YourWiFiSSID"
wifi_password: "YourWiFiPassword"

# API Encryption Key (generate with: openssl rand -base64 32)
api_encryption_key: "your-32-character-base64-key-here=="

# OTA Update Password
ota_password: "your-secure-ota-password"

# Fallback AP Password (used when WiFi fails)
fallback_password: "fallback-ap-password"
```

### 3. Pin Wiring Guide

Connect your ESP32 to the RS485 module(s):

| ESP32 Pin | RS485 Module | Description |
|-----------|--------------|-------------|
| GPIO26 | RO | UART RX - Receiver Output from RS485 |
| GPIO27 | DI | UART TX - Driver Input to RS485 |
| GPIO18 | DE & RE | Enable pin (tie DE and RE together) |
| 3.3V | VCC | Power for RS485 module |
| GND | GND | Ground |

**RS485 Module to RJ45 (MagIQTouch System):**

| RS485 Module | RJ45 Wire (A Wiring) | Purpose |
|--------------|---------------------|---------|
| A | Solid Blue | Modbus A |
| B | Orange/White | Modbus B |
| GND | Brown/White, Blue/White | Ground |
| 5V* | Solid Green, Solid Orange | Power (control panel only) |

*Note: 5V power is only needed for the control panel connection, not the system board.

### 4. Using Local Component (Development)

If you've cloned this repository locally, use this instead of `external_components`:

```yaml
external_components:
  - source:
      type: local
      path: /path/to/magiqtouch-modbus-esp32  # Path to cloned repo
    components: [magiqtouch]
```

### 5. Board-Specific Configurations

#### FireBeetle ESP32 (DFRobot)
```yaml
esp32:
  board: firebeetle32
  framework:
    type: arduino
```

#### Generic ESP32 Development Board
```yaml
esp32:
  board: esp32dev
  framework:
    type: arduino
```

#### ESP32-S3 DevKitC-1
```yaml
esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: arduino
```

#### Ethernet Boards - Pin Adjustments
For ethernet-based ESP32 boards, change the pin substitutions:

```yaml
substitutions:
  uart_rx_pin: "5"       # Different pin for ethernet boards
  uart_tx_pin: "17"      # Different pin for ethernet boards
  rs485_en_pin: "14"     # Different pin for ethernet boards
```

### 6. Installation Commands

**Using ESPHome CLI:**
```bash
# Install ESPHome
pip3 install esphome

# Compile and flash (first time via USB)
esphome run magiqtouch-hvac.yaml

# Update over-the-air (after initial flash)
esphome run magiqtouch-hvac.yaml --device <device-ip>
```

**Using Home Assistant ESPHome Dashboard:**
1. Install ESPHome add-on from Add-on Store
2. Open ESPHome dashboard
3. Click "+ NEW DEVICE"
4. Copy-paste the configuration above
5. Update secrets.yaml with your credentials
6. Click "INSTALL" and follow prompts

### 7. Troubleshooting

**Device not responding:**
- Check UART wiring (RX, TX, Enable pin)
- Verify RS485 A/B connections
- Enable VERBOSE logging: `logger: level: VERBOSE`

**CRC errors in logs:**
- Double-check RS485 A and B wiring (may be swapped)
- Ensure RS485 DE and RE are tied together
- Check ground connections

**Cannot connect to WiFi:**
- Use the fallback AP: Connect to "magiqtouch-hvac Fallback" network
- Default password is in your secrets.yaml
- Access device at 192.168.4.1

**Enable detailed logging:**
```yaml
logger:
  level: VERBOSE  # Shows all Modbus messages with hex dumps
  baud_rate: 0
```

---

### ESPHome Prerequisites
- Home Assistant with ESPHome add-on installed, OR
- ESPHome CLI installed on your computer
- ESP32 development board (see [Hardware List](Docs/HardwareList.md))
- USB cable for flashing

### ESPHome Setup Steps

1. **Prepare your secrets file:**
   - Copy `secrets.yaml.example` to `secrets.yaml`
   - Edit `secrets.yaml` with your WiFi credentials and passwords
   ```yaml
   wifi_ssid: "YourWiFiSSID"
   wifi_password: "YourWiFiPassword"
   api_encryption_key: "generate-with-openssl-rand-base64-32"
   ota_password: "choose-a-secure-password"
   fallback_password: "fallback-ap-password"
   ```

2. **Install ESPHome (if not using Home Assistant):**
   ```bash
   pip3 install esphome
   ```

3. **Compile and flash the firmware:**
   
   **Option A - Using Home Assistant ESPHome Dashboard:**
   - Open Home Assistant
   - Go to Settings → Add-ons → ESPHome
   - Click "Open Web UI"
   - Click "+ NEW DEVICE" → "CONTINUE" → "SKIP"
   - Enter a name (e.g., "magiqtouch-hvac")
   - Click "SKIP" installation
   - Replace the generated YAML content with the contents of `magiqtouch-hvac.yaml`
   - Click "INSTALL" → "Plug into this computer"
   - Follow the on-screen instructions to flash

   **Option B - Using ESPHome CLI:**
   ```bash
   # First time flash (device connected via USB)
   esphome run magiqtouch-hvac.yaml
   
   # Subsequent updates can be done over-the-air (OTA)
   esphome run magiqtouch-hvac.yaml --device <device-ip-address>
   ```

4. **Add to Home Assistant:**
   - After flashing, the device should appear automatically in Home Assistant
   - Go to Settings → Devices & Services
   - Look for "ESPHome" and click "CONFIGURE"
   - Enter the encryption key from your `secrets.yaml`
   - The device will be added with all climate controls and sensors

5. **Wire the hardware:**
   - Follow the wiring diagram below (same as Arduino version)

### ESPHome Pin Configuration

The default pin configuration for WiFi boards is:
- UART1 (Panel): RX=GPIO26, TX=GPIO27
- UART2 (Unit): RX=GPIO16, TX=GPIO17
- RS485 Enable: GPIO18

For ethernet boards, adjust the pin substitutions in `magiqtouch-hvac.yaml`:
```yaml
substitutions:
  serial1_rx_pin: "5"
  serial1_tx_pin: "17"
  serial2_rx_pin: "14"
  serial2_tx_pin: "15"
```

### 📚 Detailed ESPHome Documentation
For complete setup instructions, troubleshooting, and advanced configuration, see the **[ESPHome Setup Guide](Docs/ESPHome.md)**.

### Home Assistant Integration: 
[https://github.com/mrhteriyaki/magiqtouch-modbus-esp32-ha](https://github.com/mrhteriyaki/magiqtouch-modbus-esp32-ha)

### Tested Configuration and Limitations: [Link](Docs/Tested.md)

### Hardware List: [Link](Docs/HardwareList.md)

### API Documentation: [Link](Docs/Api.md)

---

## Arduino IDE Setup (Production-Ready)

The Arduino-based firmware is **fully functional and recommended for production use**. It's available in the `ArduinoControlLAN-AirconControl` folder.

### Configuration Steps:
- Download and Install [Arduino IDE](https://www.arduino.cc/en/software/)
- Download the folder from this repository ArduinoControlLAN-AirconControl  
(For wired ethernet esp32 boards use ArduinoControlLAN-AirconControlEthernet)
- Under Tools, Board manager install the package "esp32 by Espressif Systems"  
- Set your WiFi Network name and password you want the device to connect with in the NetworkSettings.h file. (Not required for Wired Ethernet boards)
- Set the board type in Tools -> Board -> esp32 -> Board Type   
eg: FireBeetle-ESP32 for the DF Robot Firebeetle or ESP32 Dev Module for most generic boards
- Upload the code to the ESP32 (Sketch -> Upload).
- Reset the device once complete and check serial output for the IP address assigned by the DHCP Server for successful link.  
 Ideally reserve this DHCP Lease on your router / DHCP Server so the IP does not change.
- Browse to the IP in your browser and confirm a JSON formatted response is displayed similar to [this example](Docs/Api.md).
- Wire the hardware as per diagram below (Note Ethernet boards use alternate pins)  

![diagram](Images/diagram.png)

### For ethernet boards:
IO5  / RX2 -> RS485 Module 1 TX  
IO17 / TX2 -> RS485 Module 1 RX  
IO14 -> RS485 Module 2 TX  
IO15 -> RS485 Module 2 RX  


## RS485 to RJ45 Connector Wiring
The cables from the control panel and control board use a 6P6C male connector so that would be ideal.  
However I did not have one available in testing so for this will use a regular 8P8C Wall jack.

| RS485 Module | RJ45 Wire (A Wiring) |
|--------------|-----------|
| 5V (Control Panel Port Only) | Solid Green and Solid Orange |
| GND | Brown/White and Blue/White. |
| RS485 A | Solid Blue |
| RS485 B | Orange/White |

The standard 8P8C / RJ45 female ethernet wall jack can be used to connect to the cables to the device.  
The connector to the system board only requires A,B serial wires and ground.  
The connector to the control panel requires A,B, GND and 5V to power the panel.  

The 5v power should also be connected to the 5V Pin on the ESP32, to power both RS485 modules and one RJ45 ports for the control panel.  
The pinout on the rear of an RJ45 jack can vary with A/B positions, so make sure to match up the colours not positions in the image below. 

![RJ45](Images/rj45.PNG)


Once you have wired up your device prepared, you should be able to connect it inbetween the heater unit control board and primary control panel.

If you are not replacing an existing WiFi module, you should be prompted when the module is detected to setup the WiFi settings, tap latter on this prompt.  
Try the the component configuration update if it does not show up in Settings -> Service.
[Instructions for detecting components](Docs/NewModule.md)


Here is my mess wired up inside a 3d printed housing, STL files for 3d printing available in the Housing-CAD-STL folder.  
(Image reflects older wiring with 9/10 pins, has been moved to use alternate pins since photo was taken.)  

![insidebox](Images/inside_box.jpg)

Mounted inside the return air wallspace:  
![mounted](Images/mounted.jpg)


## More Technical Notes:

### Control Panel Wiring / Pin Information: [Link](Docs/ControllerWiring.md)

### MODBUS Information [Link](Docs/Modbus.md)




