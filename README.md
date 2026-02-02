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

### ESPHome Configuration (Framework/Template)

An ESPHome configuration is provided as a **framework for developers** who want to create a native ESPHome integration. 

⚠️ **Important**: This is currently a development framework requiring a custom C++ component. **For production, use the Arduino firmware** with the [Home Assistant integration](https://github.com/mrhteriyaki/magiqtouch-modbus-esp32-ha). See [ESPHome-Status.md](ESPHome-Status.md) for details.

---

## 📋 ESPHome Framework (For Developers)

**Status**: Framework ready for development. Requires custom C++ component for Modbus protocol.

### What's Included
- Complete YAML configuration structure
- WiFi, API, and OTA setup
- UART configuration for RS485
- Entity definitions (Climate, Sensors, Switches)
- Pin configuration templates

### What's Needed
- Custom C++ component (~1000 lines) to handle MagIQTouch Modbus protocol
- See [ESPHome-Status.md](ESPHome-Status.md) for implementation details

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




