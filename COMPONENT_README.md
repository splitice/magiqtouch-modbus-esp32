# MagiqTouch ESPHome Custom Component

This document describes the `magiqtouch_component.h` custom ESPHome component that ports the MagIQTouch Modbus protocol from Arduino to ESPHome.

## Overview

The MagiqTouch component provides a full-featured climate control interface for MagIQTouch HVAC systems, supporting:
- Climate control (heating, cooling, fan modes)
- Dual-zone temperature management
- Automatic drain mode after cooling cycles
- RS485 Modbus communication with control panels and HVAC units
- Full Home Assistant integration via ESPHome

## Architecture

### Component Structure

The component is built as a single-header C++ class that inherits from:
- `esphome::Component` - For lifecycle management (setup/loop)
- `esphome::climate::Climate` - For Home Assistant climate entity integration

### Key Features Ported

1. **CRC Calculation** - Full Modbus CRC-16 implementation with lookup tables
2. **Message Processing** - Complete message relay, parsing, and validation
3. **State Management** - All HVAC state variables (power, mode, temperature, zones)
4. **Command Sending** - IoT module and control-style command generation
5. **Drain Mode** - Automatic drain cycle after 24 hours of cooling inactivity
6. **Dual UART** - Simultaneous RS485 communication with panel and unit

## Component Files

### Primary Files
- `magiqtouch_component.h` - Main component implementation (C++ header)
- `magiqtouch-hvac.yaml` - ESPHome configuration with component integration

### Message Protocol

The component implements the MagIQTouch Modbus protocol with the following message types:

#### IoT Module Messages
- **Info Request** (0xEB 0x03 0x03 0xE4...) - MAC address query
- **Status Request** (0xEB 0x03 0x02 0x58...) - WiFi status
- **Command Request** (0xEB 0x03 0x0B 0x0C...) - HVAC command updates
- **Power State** (0xEB 0x10 0x08 0xE5...) - System on/off state
- **Info Update** (0xEB 0x10 0x08 0xE6...) - Full system status (109 bytes)

#### Control Messages
- **Control Command** (0x02 0x10 0x00 0x31...) - Direct fan/cooler control

#### Panel Messages
- **Panel 2 Info** (0x97 0x03 0x02...) - Zone 2 temperature sensor

## Integration with ESPHome YAML

### Climate Entity

The component provides a climate entity with:
- **Modes**: Off, Fan Only, Cool, Heat
- **Fan Modes**: Low, Medium, High, Auto
- **Temperature Control**: 10°C - 28°C
- **Current Temperature**: From thermister sensor

### Sensors Exposed

| Sensor | Type | Description |
|--------|------|-------------|
| Command Info | Number | Panel command counter |
| Evap Fan Speed | Number | Evaporator fan speed (0-10) |
| Heater Fan Speed | Number | Heater fan speed (0-10) |
| Zone 1 Temperature | Temperature | Zone 1 temp sensor reading |
| Zone 2 Temperature | Temperature | Zone 2 temp sensor reading |
| Thermister Temperature | Temperature | Main thermister reading |
| Target Temp Zone 2 | Temperature | Zone 2 target temperature |
| Zone 1 Enabled | Binary | Zone 1 heating status |
| Zone 2 Enabled | Binary | Zone 2 heating status |
| Drain Mode Active | Binary | Drain cycle active |
| Automatic Clean | Binary | Self-cleaning mode active |
| System Mode | Text | Current operating mode |

### Controls

| Control | Type | Description |
|---------|------|-------------|
| Zone 1 Switch | Switch | Enable/disable Zone 1 heating |
| Zone 2 Switch | Switch | Enable/disable Zone 2 heating |
| Target Temp Zone 2 | Number | Set Zone 2 target (10-28°C) |
| Trigger Drain Mode | Button | Manually start drain cycle |
| Cancel Drain Mode | Button | Cancel active drain cycle |

## System Modes

The component supports 5 system modes:

| Mode | Value | Description |
|------|-------|-------------|
| Fan External | 0 | External fan only |
| Fan Recycle | 1 | Recycle fan mode |
| Cooler Manual | 2 | Manual cooling mode |
| Cooler Auto | 3 | Auto cooling with temp control |
| Heater | 4 | Heating mode with zone control |

## Drain Mode Operation

### Purpose
After cooling operation, condensation accumulates in the evaporator. Drain mode runs the fan to dry out the unit and prevent mold/bacteria growth.

### Timing
- **Trigger**: 24 hours after cooling mode ends
- **Duration**: 2 minutes
- **Manual**: Can be triggered/cancelled via button
- **Auto-cancel**: Resets if cooling resumes

### Implementation
```cpp
bool update_drain_mode() {
  // Track cooling mode transitions
  // Start drain after COOL_TRANSITION_TO_DRAIN (24h)
  // Run for DRAIN_TIME (2min)
  // Support manual trigger/cancel
}
```

## UART Configuration

### Hardware Setup
- **UART 1 (Panel)**: GPIO26 (RX), GPIO27 (TX)
- **UART 2 (Unit)**: GPIO16 (RX), GPIO17 (TX)
- **RS485 Enable**: GPIO18 (DE/RE control)
- **Baud Rate**: 9600, 8N1

### Message Flow
```
Control Panel <--RS485--> ESP32 <--RS485--> HVAC Unit
                         (relay + intercept)
```

The ESP32 acts as a transparent proxy, relaying messages between panel and unit while:
1. Intercepting state updates
2. Injecting command messages
3. Responding to IoT module queries

## CRC Calculation

Standard Modbus CRC-16 with lookup tables for performance:

```cpp
uint16_t modbus_crc(uint8_t *buffer, uint16_t buffer_length) {
  uint8_t crc_hi = 0xFF;
  uint8_t crc_lo = 0xFF;
  
  while (buffer_length--) {
    i = crc_hi ^ *buffer++;
    crc_hi = crc_lo ^ table_crc_hi[i];
    crc_lo = table_crc_lo[i];
  }
  
  return (crc_hi << 8 | crc_lo);
}
```

## State Management

### Thread Safety
ESPHome is single-threaded, so no mutex/semaphore required (unlike Arduino version).

### State Variables
- **Control State**: `system_power_`, `fan_speed_`, `system_mode_`, `target_temp_`
- **Info State**: `*_info_` variables track actual system state
- **Zone State**: `heater_zone1_`, `heater_zone2_`, `target_temp2_`
- **Drain State**: `drain_mode_active_`, `drain_mode_manual_`, timing variables

### State Synchronization
- **From Panel**: `iot_module_message_process()` updates state from panel commands
- **To System**: `send_command_message()` / `send_command_control()` sends commands
- **To HA**: `publish_sensors()` / `publish_state()` updates Home Assistant

## IoT Support Detection

The component supports both IoT module and direct control modes:

```cpp
if (!iot_supported_) {
  send_command_control(drain_active);  // Direct control commands
  return;
}
// IoT module commands
send_command_message();
```

Detection happens automatically based on message patterns received.

## Command Message Format

### IoT Module Command (29 bytes)
```
0xEB 0x03 0x1A 0x02 [counter] [wifi] [power] [evap_mode]
[evap_fan] [unknown] [heater_fan] 0x00 [zone_temp] 0x00
[zone_temp] 0x00 [zone_control] 0x00 0x00 [temp_z2] [temp_z1]
0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 [CRC_HI] [CRC_LO]
```

### Control Command (11 bytes)
```
0x02 0x10 0x00 0x31 0x00 0x01 0x02 0x00 [XX] [CRC_HI] [CRC_LO]

XX = (fan_speed << 4) | flags
  flags: bit 1 = cooler mode, bit 0 = drain mode
```

## Building and Flashing

### Prerequisites
```bash
pip install esphome
```

### Compile
```bash
esphome compile magiqtouch-hvac.yaml
```

### Flash (USB)
```bash
esphome upload magiqtouch-hvac.yaml
```

### Flash (OTA)
```bash
esphome upload magiqtouch-hvac.yaml --device magiqtouch-hvac.local
```

## Differences from Arduino Version

| Feature | Arduino | ESPHome |
|---------|---------|---------|
| Threading | FreeRTOS tasks | Single-threaded loop |
| Mutex/Semaphore | Required | Not needed |
| Web Server | Custom WebServer | Built-in ESPHome API |
| JSON API | ArduinoJson | Native HA integration |
| Serial Debug | Serial.println | ESP_LOG macros |
| Network | WiFi + LAN option | WiFi only |
| State Persistence | None | ESPHome restore_state |

## Debugging

### Enable Verbose Logging
```yaml
logger:
  level: VERBOSE
  logs:
    magiqtouch: VERBOSE
```

### Log Levels
- `ESP_LOGI` - Informational (mode changes, drain events)
- `ESP_LOGD` - Debug (command details)
- `ESP_LOGV` - Verbose (message reception)
- `ESP_LOGW` - Warnings (buffer overflows)

## Troubleshooting

### No Communication
1. Check UART pin configuration matches hardware
2. Verify RS485 enable pin is correct
3. Check baud rate (should be 9600)
4. Inspect RS485 transceiver wiring

### Commands Not Working
1. Check `iot_supported_` detection
2. Verify CRC calculation
3. Enable verbose logging to see messages
4. Check state synchronization in `publish_sensors()`

### Drain Mode Not Triggering
1. Verify cooling mode was active
2. Check 24-hour timer hasn't reset (power cycle)
3. Ensure system transitions out of cooling mode
4. Try manual trigger button

## License

This component is derived from the Arduino MagIQTouch implementation and maintains compatibility with the original project's architecture and protocol implementation.

## Credits

- Original Arduino implementation: [ArduinoControlLAN-AirconControl](ArduinoControlLAN-AirconControl/)
- ESPHome framework: https://esphome.io
- Home Assistant: https://www.home-assistant.io
