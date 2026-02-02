# MagiqTouch ESPHome Custom Component

This document describes the `magiqtouch_component.h` custom ESPHome component that ports the MagIQTouch Modbus protocol from Arduino to ESPHome.

## Overview

The MagiqTouch component provides a full-featured climate control interface for MagIQTouch HVAC systems, supporting:
- Climate control (heating, cooling, fan modes)
- Dual-zone temperature management
- Automatic drain mode after cooling cycles
- RS485 Modbus communication with control panels and HVAC units
- Full Home Assistant integration via ESPHome
- **HTTP REST API for backward compatibility**
- **Comprehensive debug logging**

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
7. **HTTP REST API** - Arduino-compatible REST endpoints
8. **Debug Logging** - ESPHome logger integration with multiple log levels

## HTTP REST API

The component includes built-in HTTP API handlers for backward compatibility with the Arduino firmware:

### GET / - Status Endpoint

Returns JSON status of the HVAC system:

```bash
curl http://<device-ip>/
```

Response format:
```json
{
  "module_name": "ESP32-HVAC-Control-ESPHome",
  "uptime": "0d 01:23:45",
  "system_power": 1,
  "system_mode": 2,
  "target_temp": 22,
  "target_temp_zone2": 20,
  "evap_mode": 0,
  "evap_fanspeed": 5,
  "heater_mode": 0,
  "heater_fanspeed": 0,
  "heater_therm_temp": 24,
  "heater_zone1_enabled": 1,
  "heater_zone2_enabled": 1,
  "zone1_temp_sensor": 23,
  "zone2_temp_sensor": 22,
  "panel_command_count": 42,
  "automatic_clean_running": 0,
  "drain_mode_active": false,
  "drain_time_remaining_ms": 0,
  "time_until_next_drain_ms": 0
}
```

### POST /command - Control Endpoint

Send commands to control the HVAC system:

```bash
# Power control
curl -X POST http://<device-ip>/command -d "power=on"
curl -X POST http://<device-ip>/command -d "power=off"

# Zone control
curl -X POST http://<device-ip>/command -d "zone1=on"
curl -X POST http://<device-ip>/command -d "zone2=off"

# Drain mode
curl -X POST http://<device-ip>/command -d "drain=on"
curl -X POST http://<device-ip>/command -d "drain=off"

# Fan speed (0-10)
curl -X POST http://<device-ip>/command -d "fanspeed=7"

# System mode (0=Fan Ext, 1=Fan Recycle, 2=Cooler Manual, 3=Cooler Auto, 4=Heater)
curl -X POST http://<device-ip>/command -d "mode=2"

# Temperature (10-28°C)
curl -X POST http://<device-ip>/command -d "temp=22"
curl -X POST http://<device-ip>/command -d "temp2=20"
```

**Note:** ESPHome's built-in web server (v2) also provides entity REST API at standard endpoints. The custom HTTP API methods (`get_json_status()`, `handle_command()`) are available for programmatic access within the component.

## Debug Logging

The component uses ESPHome's logging system with multiple log levels:

### Log Levels

- **ERROR** - Critical errors only
- **WARN** - Warnings (CRC mismatches, buffer overflows)
- **INFO** - Important events (mode changes, drain mode activation, HTTP API calls)
- **DEBUG** - Detailed operation (message processing, command sends, control changes)
- **VERBOSE** - All message traffic with hex dumps

### Enable Debug Logging

Edit `magiqtouch-hvac.yaml`:

```yaml
logger:
  level: DEBUG  # or VERBOSE for maximum detail
  baud_rate: 0  # Keep 0 to avoid UART conflicts
```

### Log Output Examples

```
[I][magiqtouch:327] HTTP API: System Power ON
[D][magiqtouch:709] Control command: fan=5, cooler=1
[D][magiqtouch:1035] Sending message (11 bytes): 02 10 00 31 00 01 02 00 52 [CRC:B3C4]
[V][magiqtouch:678] Valid message received on serial 1, length 8
[V][magiqtouch:684] Message data: EB 03 03 E4 00 05 D3 70
[W][magiqtouch:695] CRC mismatch on serial 2: got 0x1234, expected 0x5678
[I][magiqtouch:790] Cooling mode ended, drain timer started
[I][magiqtouch:801] Auto-drain mode activated after 24-hour idle period
```

### Viewing Logs

**Via ESPHome Dashboard:**
- Click "LOGS" button for your device
- Logs stream in real-time

**Via ESPHome CLI:**
```bash
esphome logs magiqtouch-hvac.yaml
```

**Via Home Assistant:**
- Settings → System → Logs
- Filter by "magiqtouch"

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
- **Current Temperature**: From thermistor sensor

### Sensors Exposed

| Sensor | Type | Description |
|--------|------|-------------|
| Command Info | Number | Panel command counter |
| Evap Fan Speed | Number | Evaporator fan speed (0-10) |
| Heater Fan Speed | Number | Heater fan speed (0-10) |
| Zone 1 Temperature | Temperature | Zone 1 temp sensor reading |
| Zone 2 Temperature | Temperature | Zone 2 temp sensor reading |
| Thermistor Temperature | Temperature | Main thermistor reading |
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
