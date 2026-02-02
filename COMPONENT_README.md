# MagiqTouch ESPHome Custom Component

This document describes the simplified `magiqtouch_component.h` custom ESPHome component for MagIQTouch HVAC control.

## Overview

Simplified climate control interface for MagIQTouch HVAC systems:
- Climate control (heating, cooling, fan modes)
- Dual-zone temperature management  
- Automatic drain mode after cooling cycles
- **Single UART RS485 Modbus** (simplified from dual-relay mode)
- Full Home Assistant integration via ESPHome
- **ESPHome buttons/switches for all controls** (replaces HTTP API)
- **No IoT module support** (!IOTSupported mode only)
- Comprehensive debug logging

## Simplified Design

This version removes unnecessary complexity for cleaner integration:
- **Single UART**: One serial port for Modbus network (not dual relay)
- **No IoT Module**: Only direct control commands (!IOTSupported mode)
- **No HTTP API**: Control via native ESPHome buttons/switches
- **Smaller**: ~580 lines vs ~1100 lines in complex version

## Control Interface

All HVAC control through ESPHome entities:

### Buttons
- Power ON / Power OFF
- Mode: Fan External, Fan Recycle, Cooler Manual, Cooler Auto, Heater
- Trigger Drain Mode / Cancel Drain Mode

### Switches
- Zone 1 Enable/Disable
- Zone 2 Enable/Disable

### Number
- Fan Speed (0-10 slider)

### Climate Entity
- Main thermostat interface
- Temperature setpoint
- Mode selection
- Fan control

## Hardware Configuration

Default pins (customizable in YAML):
```yaml
uart_rx_pin: "26"   # UART RX
uart_tx_pin: "27"   # UART TX  
rs485_en_pin: "18"  # RS485 enable
```

## Installation

1. Copy `magiqtouch-hvac.yaml` and `magiqtouch_component.h` to ESPHome config
2. Configure `secrets.yaml` with WiFi credentials
3. Flash: `esphome run magiqtouch-hvac.yaml`
4. Device auto-discovers in Home Assistant

## Debug Logging

Enable in `magiqtouch-hvac.yaml`:
```yaml
logger:
  level: DEBUG  # or VERBOSE for hex dumps
```

View logs via ESPHome Dashboard, CLI (`esphome logs`), or Home Assistant logs.
