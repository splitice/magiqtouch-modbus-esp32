#include "magiqtouch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace magiqtouch {

static const char *const TAG = "magiqtouch";

// CRC Lookup Tables (Modbus CRC-16)
static const uint8_t table_crc_hi[] = {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

static const uint8_t table_crc_lo[] = {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
  0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
  0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
  0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
  0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
  0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
  0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
  0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
  0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
  0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
  0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
  0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
  0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
  0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
  0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
  0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
  0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
  0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
  0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
  0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
  0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
  0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
  0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
  0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

void MagiqTouchComponent::setup() {
  if (this->rs485_en_pin_ != nullptr) {
    this->rs485_en_pin_->setup();
    this->rs485_en_pin_->pin_mode(gpio::FLAG_OUTPUT);
    this->rs485_en_pin_->digital_write(false);  // Receiving mode by default
  }

  this->update_sensors();
  
  ESP_LOGD(TAG, "MagIQTouch component setup complete");
}

void MagiqTouchComponent::loop() {
  // Process incoming UART data
  this->process_uart();
  
  // Update drain mode logic
  this->update_drain_mode();
  
  // Send control commands periodically (every 500ms)
  uint32_t current_millis = millis();
  if (current_millis - this->last_command_send_ >= 500) {
    this->send_command_control(this->drain_mode_active_);
    this->last_command_send_ = current_millis;
  }
}

void MagiqTouchComponent::set_fan_speed(uint8_t speed) {
  ESP_LOGD(TAG, "Setting fan speed to %d", speed);
  this->fan_speed_ = speed;
}

void MagiqTouchComponent::set_fan_speed_str(const std::string &speed_str) {
  auto speed_opt = parse_number<uint8_t>(speed_str);
  if (!speed_opt.has_value()) {
    ESP_LOGW(TAG, "Invalid fan speed string: %s", speed_str.c_str());
    return;
  }
  
  uint8_t speed = speed_opt.value();
  if (speed > 10) {
    ESP_LOGW(TAG, "Fan speed out of range (0-10): %d", speed);
    return;
  }
  
  ESP_LOGI(TAG, "Setting fan speed from string '%s' to %d", speed_str.c_str(), speed);
  this->set_fan_speed(speed);
}

void MagiqTouchComponent::set_power(bool power) {
  ESP_LOGD(TAG, "Setting power to %s", power ? "ON" : "OFF");
  this->system_power_ = power;

  this->update_sensors();
}

void MagiqTouchComponent::set_mode(uint8_t mode) {
  ESP_LOGD(TAG, "Setting mode to %d", mode);
  this->system_mode_ = mode;
  
  // Track when cooling mode ends for drain mode automation
  if (this->last_system_mode_ == 2 && mode != 2) {  // Exiting cool mode
    this->last_cool_mode_end_time_ = millis();
  }
  this->last_system_mode_ = mode;

  this->update_sensors();
}

void MagiqTouchComponent::set_mode_by_name(const std::string &mode_name) {
  ESP_LOGI(TAG, "Setting mode by name: %s", mode_name.c_str());
  
  if (mode_name == "off") {
    this->set_power(false);
  } else if (mode_name == "fan_only") {
    this->set_power(true);
    this->set_mode(0);  // Fan External mode
  } else if (mode_name == "cool") {
    this->set_power(true);
    this->set_mode(2);  // Cooler Manual mode
  } else if (mode_name == "heat") {
    this->set_power(true);
    this->set_mode(4);  // Heater mode
  } else {
    ESP_LOGW(TAG, "Unknown mode name: %s", mode_name.c_str());
    return;
  }
  
  // Send the control command immediately
  this->send_command_control(this->drain_mode_active_);
}

std::string MagiqTouchComponent::get_mode_name() const {
  if (!this->system_power_) {
    return "off";
  }
  
  // Map numeric mode to mode name
  if (this->system_mode_ <= 1) {
    return "fan_only";  // Fan External (0) or Fan Recycle (1)
  } else if (this->system_mode_ <= 3) {
    return "cool";  // Cooler Manual (2) or Cooler Auto (3)
  } else {
    return "heat";  // Heater (4)
  }
}

void MagiqTouchComponent::trigger_drain_mode() {
  ESP_LOGI(TAG, "Drain mode triggered manually");
  this->drain_mode_manual_ = true;
  this->drain_mode_active_ = true;
  this->drain_mode_start_time_ = millis();
  this->update_sensors();
}

void MagiqTouchComponent::cancel_drain_mode() {
  ESP_LOGI(TAG, "Drain mode cancelled");
  this->drain_mode_manual_ = false;
  this->drain_mode_active_ = false;
  this->update_sensors();
}

void MagiqTouchComponent::process_uart() {
  uint32_t current_millis = millis();
  
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    
    // Check for gap (message boundary)
    if (current_millis - this->previous_millis_ > GAPTHRESHOLD) {
      this->serial_index_ = 0;  // Reset buffer on gap
    }
    this->previous_millis_ = current_millis;
    
    // Add byte to buffer
    if (this->serial_index_ < MAXMSGSIZE) {
      this->serial_buffer_[this->serial_index_++] = byte;
    } else {
      ESP_LOGW(TAG, "Serial buffer overflow");
      this->serial_index_ = 0;
    }
    
    // Try to process message if we have enough bytes
    if (this->serial_index_ >= 8) {  // Minimum message size
      if (this->process_message(this->serial_buffer_, this->serial_index_)) {
        this->serial_index_ = 0;  // Message processed, reset buffer
      }
    }
  }
}

bool MagiqTouchComponent::process_message(uint8_t *msg_buffer, int msg_length) {
  // Check for control command pattern (0x02 0x10...)
  if (msg_length >= 11 && msg_buffer[0] == 0x02 && msg_buffer[1] == 0x10) {
    // Verify CRC - Arduino sends HIGH byte first, then LOW byte
    uint16_t received_crc = (msg_buffer[msg_length - 2] << 8) | msg_buffer[msg_length - 1];
    uint16_t calculated_crc = this->modbus_crc(msg_buffer, msg_length - 2);
    
    if (received_crc == calculated_crc) {
      ESP_LOGV(TAG, "Valid control message received");
      this->control_command_process(msg_buffer, msg_length);
      return true;
    } else {
      ESP_LOGW(TAG, "CRC mismatch: expected 0x%04X, got 0x%04X", calculated_crc, received_crc);
    }
  }
  
  return false;
}

void MagiqTouchComponent::control_command_process(uint8_t *msg_buffer, int msg_length) {
  if (msg_length < 11) return;
  
  // Extract control byte (byte 8)
  uint8_t control_byte = msg_buffer[8];
  
  // Parse system state from control byte using Arduino logic
  // Control byte format: (FanSpeed << 4) + modifiers
  // modifiers: +2 for cooler mode or drain, +1 additional for drain
  // 0x00 = power off
  
  bool power = (control_byte != 0x00);
  uint8_t fan = (control_byte >> 4) & 0x0F;  // Extract fan speed from upper nibble
  uint8_t lower_nibble = control_byte & 0x0F;
  
  // Determine mode and drain based on lower nibble
  // 0x00-0x01: Fan mode (External or Recycle)
  // 0x02: Cooler mode 
  // 0x03: Cooler mode with drain
  bool drain_detected = (lower_nibble == 0x03 || lower_nibble == 0x01);
  bool cooler_detected = (lower_nibble == 0x02 || lower_nibble == 0x03);
  
  ESP_LOGD(TAG, "Received control byte: 0x%02X - Power: %s, Fan: %d, LowerNibble: 0x%01X", 
           control_byte, power ? "ON" : "OFF", fan, lower_nibble);
  
  // Note: Without IoT module, we don't actually receive state updates from the system.
  // This parsing is here for debugging/monitoring if we see messages on the bus.
  // We primarily control state through our own commands, not by reading the bus.
}

void MagiqTouchComponent::update_drain_mode() {
  uint32_t current_millis = millis();
  
  // Auto-trigger drain mode 24h after leaving cool mode
  if (!this->drain_mode_active_ && !this->drain_mode_manual_) {
    if (this->last_cool_mode_end_time_ > 0 && 
        current_millis - this->last_cool_mode_end_time_ >= COOL_TRANSITION_TO_DRAIN) {
      ESP_LOGI(TAG, "Auto-triggering drain mode (24h after cool mode)");
      this->drain_mode_active_ = true;
      this->drain_mode_start_time_ = current_millis;
      this->update_sensors();
      return;
    }
  }
  
  // End drain mode after 2 minutes
  if (this->drain_mode_active_) {
    if (current_millis - this->drain_mode_start_time_ >= DRAIN_TIME) {
      ESP_LOGI(TAG, "Drain mode complete");
      this->drain_mode_active_ = false;
      this->drain_mode_manual_ = false;
      this->update_sensors();
    }
  }
}

void MagiqTouchComponent::send_command_control(bool drain_active) {
  // Format: 02 02 10 00 31 00 01 02 00 XX YY YY
  // Where XX = (FanSpeed * 16) + 2 if in cooler mode, YY YY is CRC.
  // Drain mode uses FanSpeed + 1 (which becomes (FanSpeed+1) * 16 in the upper nibble)
  
  uint8_t command[11];
  
  // Copy command header: { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00 }
  memcpy(command, CONTROL_COMMAND_HEADER, 8);
  
  // Build control byte using Arduino logic
  const bool cooler_mode = (this->system_mode_ == 2) || (this->system_mode_ == 3);
  uint8_t effective_fan_speed = this->fan_speed_;
  
  uint8_t xx = (uint8_t)(effective_fan_speed << 4);  // Fan speed in upper nibble
  if (cooler_mode) {
    xx = (uint8_t)(xx + 2);  // Add 2 if cooler mode or drain
  }
  if (!this->system_power_) {
    xx = 0x00;  // Power off command
  }
  
  if (drain_active) {
    xx += 1;  // Add 1 if drain active (so drain mode is +3 total in cooler)
  }
  
  command[8] = xx;
  
  // Add CRC and send
  this->send_message(command, 9, true);
  
  ESP_LOGD(TAG, "Sent control command - Power: %s, Mode: %d, Fan: %d, Drain: %s, ControlByte: 0x%02X",
           this->system_power_ ? "ON" : "OFF", this->system_mode_, this->fan_speed_,
           drain_active ? "YES" : "NO", xx);
}

void MagiqTouchComponent::send_message(uint8_t *msg_buffer, int length, bool send_crc) {
  if (send_crc) {
    uint16_t crc = this->modbus_crc(msg_buffer, length);
    msg_buffer[length] = (crc >> 8) & 0xFF;     // CRC high byte first
    msg_buffer[length + 1] = crc & 0xFF;        // CRC low byte second
    length += 2;
  }
  
  // Enable transmit mode
  if (this->rs485_en_pin_ != nullptr) {
    this->rs485_en_pin_->digital_write(true);
    delayMicroseconds(100);  // Small delay for RS485 transceiver
  } else {
    ESP_LOGW(TAG, "RS485 EN pin not configured, cannot send message");
    return;
  }
  
  // Send message
  this->write_array(msg_buffer, length);
  this->flush();
  
  // Wait for transmission to complete
  delayMicroseconds(500);
  
  // Return to receive mode
  this->rs485_en_pin_->digital_write(false);
  
  ESP_LOGV(TAG, "Sent message (%d bytes) with CRC: %d", length,
           send_crc ? (uint16_t)(msg_buffer[length - 2] << 8 | msg_buffer[length - 1]) : 0);
  }

uint16_t MagiqTouchComponent::modbus_crc(uint8_t *buffer, int length) {
  uint8_t crc_hi = 0xFF;
  uint8_t crc_lo = 0xFF;
  
  for (int i = 0; i < length; i++) {
    int index = crc_hi ^ buffer[i];
    crc_hi = crc_lo ^ table_crc_hi[index];
    crc_lo = table_crc_lo[index];
  }
  
  return (crc_hi << 8) | crc_lo;
}

bool MagiqTouchComponent::check_pattern(const uint8_t *buffer, const uint8_t *pattern, size_t length) {
  return memcmp(buffer, pattern, length) == 0;
}

void MagiqTouchComponent::update_sensors() {
  // Update drain mode binary sensor
  if (this->drain_mode_active_sensor_ != nullptr) {
    this->drain_mode_active_sensor_->publish_state(this->drain_mode_active_);
  }
  
  // Update system mode text sensor
  if (this->system_mode_sensor_ != nullptr) {
    const char *mode_text = "Unknown";
    if(!this->system_power_) {
      mode_text = "off";
    } else {
      switch (this->system_mode_) {
        case 0:
        case 1:
          mode_text = "fan_only";
          break;
        case 2:
        case 3:
          mode_text = "cool";
          break;
        case 4:
          mode_text = "heat"; 
          break;
      }
    }
    this->system_mode_sensor_->publish_state(mode_text);
  }
}

}  // namespace magiqtouch
}  // namespace esphome
