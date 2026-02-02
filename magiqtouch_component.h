#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace magiqtouch {

// Constants
static const uint8_t MAXMSGSIZE = 256;
static const uint8_t GAPTHRESHOLD = 50;
static const uint32_t DRAIN_TIME = 2 * 60 * 1000UL;  // 2 minutes
static const uint32_t COOL_TRANSITION_TO_DRAIN = 24 * 60 * 60 * 1000UL;  // 24 hours

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

// Control command header (no IoT module support)
static const uint8_t CONTROL_COMMAND_HEADER[] = { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00 };

class MagiqTouchClimate : public climate::Climate, public Component {
 public:
  MagiqTouchClimate() {}

  void setup() override {
    // Initialize state variables
    this->system_power_ = false;
    this->fan_speed_ = 5;
    this->system_mode_ = 0;
    this->target_temp_ = 20;
    this->heater_zone1_ = false;
    this->heater_zone2_ = false;
    
    // Drain mode initialization
    this->drain_mode_manual_ = false;
    this->drain_mode_active_ = false;
    this->last_cool_mode_end_time_ = 0;
    this->drain_mode_start_time_ = 0;
    this->last_system_mode_ = 0;
    
    // Serial buffer initialization
    this->serial_index_ = 0;
    this->previous_millis_ = 0;
    
    // Set up climate traits
    this->traits_.set_supports_current_temperature(true);
    this->traits_.set_supports_two_point_target_temperature(false);
    this->traits_.set_visual_min_temperature(10);
    this->traits_.set_visual_max_temperature(28);
    this->traits_.set_visual_temperature_step(1);
    
    // Supported modes
    this->traits_.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_FAN_ONLY,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT,
    });
    
    // Supported fan modes
    this->traits_.set_supported_fan_modes({
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
      climate::CLIMATE_FAN_AUTO,
    });
    
    // Restore state from flash
    auto restore = this->restore_state_();
    if (restore.has_value()) {
      restore->apply(this);
    }
    
    ESP_LOGD("magiqtouch", "MagiqTouch Climate component setup complete");
  }

  void loop() override {
    uint32_t now = millis();
    
    // Process UART data
    this->process_uart();
    
    // Update drain mode (called periodically in loop)
    if (now - this->last_drain_update_ > 1000) {  // Check every second
      this->update_drain_mode();
      this->last_drain_update_ = now;
    }
    
    // Send periodic command (every 5 seconds)
    if (now - this->last_command_send_ > 5000) {
      this->send_command_control(this->drain_mode_active_);
      this->last_command_send_ = now;
    }
  }

  void set_uart(uart::UARTComponent *uart) { this->uart_ = uart; }
  void set_rs485_enable_pin(GPIOPin *pin) { this->rs485_en_pin_ = pin; }
  
  // Sensor setters
  void set_zone1_temp_sensor(sensor::Sensor *sensor) { this->zone1_temp_sensor_ = sensor; }
  void set_zone2_temp_sensor(sensor::Sensor *sensor) { this->zone2_temp_sensor_ = sensor; }
  void set_thermistor_temp_sensor(sensor::Sensor *sensor) { this->thermistor_temp_sensor_ = sensor; }
  
  // Binary sensor setters
  void set_zone1_enabled_sensor(binary_sensor::BinarySensor *sensor) { this->zone1_enabled_sensor_ = sensor; }
  void set_zone2_enabled_sensor(binary_sensor::BinarySensor *sensor) { this->zone2_enabled_sensor_ = sensor; }
  void set_drain_mode_active_sensor(binary_sensor::BinarySensor *sensor) { this->drain_mode_active_sensor_ = sensor; }
  
  // Text sensor setters
  void set_system_mode_sensor(text_sensor::TextSensor *sensor) { this->system_mode_sensor_ = sensor; }

  // Climate interface overrides
  climate::ClimateTraits traits() override { return this->traits_; }

  void control(const climate::ClimateCall &call) override {
    if (call.get_mode().has_value()) {
      climate::ClimateMode mode = *call.get_mode();
      
      switch (mode) {
        case climate::CLIMATE_MODE_OFF:
          this->system_power_ = false;
          break;
        case climate::CLIMATE_MODE_FAN_ONLY:
          this->system_power_ = true;
          this->system_mode_ = 0;  // Fan - External
          break;
        case climate::CLIMATE_MODE_COOL:
          this->system_power_ = true;
          this->system_mode_ = 2;  // Cooler - Manual
          break;
        case climate::CLIMATE_MODE_HEAT:
          this->system_power_ = true;
          this->system_mode_ = 4;  // Heater
          break;
        default:
          break;
      }
      
      this->mode = mode;
    }
    
    if (call.get_target_temperature().has_value()) {
      this->target_temp_ = (uint8_t)*call.get_target_temperature();
      this->target_temperature = *call.get_target_temperature();
    }
    
    if (call.get_fan_mode().has_value()) {
      climate::ClimateFanMode fan_mode = *call.get_fan_mode();
      
      switch (fan_mode) {
        case climate::CLIMATE_FAN_LOW:
          this->fan_speed_ = 3;
          break;
        case climate::CLIMATE_FAN_MEDIUM:
          this->fan_speed_ = 6;
          break;
        case climate::CLIMATE_FAN_HIGH:
          this->fan_speed_ = 9;
          break;
        case climate::CLIMATE_FAN_AUTO:
          this->fan_speed_ = 5;
          break;
        default:
          break;
      }
      
      this->fan_mode = fan_mode;
    }
    
    // Trigger command send immediately
    this->send_command_control(this->drain_mode_active_);
    
    this->publish_state();
  }

  // Additional control methods (for buttons)
  void set_zone1(bool enabled) {
    ESP_LOGI("magiqtouch", "Zone 1 %s", enabled ? "ON" : "OFF");
    this->heater_zone1_ = enabled;
    if (this->zone1_enabled_sensor_ != nullptr) {
      this->zone1_enabled_sensor_->publish_state(enabled);
    }
  }
  
  void set_zone2(bool enabled) {
    ESP_LOGI("magiqtouch", "Zone 2 %s", enabled ? "ON" : "OFF");
    this->heater_zone2_ = enabled;
    if (this->zone2_enabled_sensor_ != nullptr) {
      this->zone2_enabled_sensor_->publish_state(enabled);
    }
  }
  
  void set_fan_speed(uint8_t speed) {
    if (speed >= 0 && speed <= 10) {
      ESP_LOGI("magiqtouch", "Fan speed set to %d", speed);
      this->fan_speed_ = speed;
      this->send_command_control(this->drain_mode_active_);
    }
  }
  
  void set_power(bool power) {
    ESP_LOGI("magiqtouch", "System power %s", power ? "ON" : "OFF");
    this->system_power_ = power;
    this->send_command_control(this->drain_mode_active_);
    this->publish_state();
  }
  
  void set_mode(uint8_t mode) {
    if (mode >= 0 && mode <= 5) {
      ESP_LOGI("magiqtouch", "System mode set to %d", mode);
      this->system_mode_ = mode;
      this->send_command_control(this->drain_mode_active_);
      this->publish_state();
    }
  }
  
  void trigger_drain_mode() {
    this->drain_mode_manual_ = true;
    ESP_LOGI("magiqtouch", "Manual drain mode triggered");
  }
  
  void cancel_drain_mode() {
    this->drain_mode_active_ = false;
    this->drain_mode_manual_ = false;
    ESP_LOGI("magiqtouch", "Drain mode cancelled");
  }

 protected:
  // UART component (single port for modbus network)
  uart::UARTComponent *uart_{nullptr};
  GPIOPin *rs485_en_pin_{nullptr};
  
  // Climate traits
  climate::ClimateTraits traits_;
  
  // Control variables
  bool system_power_;
  uint8_t fan_speed_;
  uint8_t system_mode_;
  uint8_t target_temp_;
  bool heater_zone1_;
  bool heater_zone2_;
  
  // Drain mode variables
  bool drain_mode_manual_;
  bool drain_mode_active_;
  uint32_t last_cool_mode_end_time_;
  uint32_t drain_mode_start_time_;
  uint8_t last_system_mode_;
  
  // Timing variables
  uint32_t last_drain_update_{0};
  uint32_t last_command_send_{0};
  
  // Serial buffer
  uint8_t serial_buffer_[MAXMSGSIZE];
  int serial_index_;
  uint32_t previous_millis_;
  
  // Sensor pointers
  sensor::Sensor *zone1_temp_sensor_{nullptr};
  sensor::Sensor *zone2_temp_sensor_{nullptr};
  sensor::Sensor *thermistor_temp_sensor_{nullptr};
  
  // Binary sensor pointers
  binary_sensor::BinarySensor *zone1_enabled_sensor_{nullptr};
  binary_sensor::BinarySensor *zone2_enabled_sensor_{nullptr};
  binary_sensor::BinarySensor *drain_mode_active_sensor_{nullptr};
  
  // Text sensor pointers
  text_sensor::TextSensor *system_mode_sensor_{nullptr};

  // Process incoming UART data
  void process_uart() {
    if (this->uart_ == nullptr) return;
    
    while (this->uart_->available()) {
      uint8_t incoming_byte;
      this->uart_->read_byte(&incoming_byte);
      
      // Gap threshold - mark message complete
      uint32_t current_millis = millis();
      if (this->serial_index_ > 0 && current_millis - this->previous_millis_ > GAPTHRESHOLD) {
        this->serial_index_ = 0;
      }
      this->previous_millis_ = current_millis;
      
      if (this->serial_index_ == MAXMSGSIZE) {
        ESP_LOGW("magiqtouch", "Buffer limit reached on UART");
        this->serial_index_ = 0;
      }
      
      // Save byte to buffer
      this->serial_buffer_[this->serial_index_++] = incoming_byte;
      if (this->process_message(this->serial_buffer_, this->serial_index_)) {
        this->serial_index_ = 0;
      }
    }
  }

  bool process_message(uint8_t *msg_buffer, int msg_length) {
    if (msg_length < 4) {
      return false;
    }
    
    // CRC validation
    uint16_t crc_raw = (msg_buffer[msg_length - 2] << 8 | msg_buffer[msg_length - 1]);
    uint16_t expected_crc = this->modbus_crc(msg_buffer, msg_length - 2);
    
    if (crc_raw == expected_crc) {
      ESP_LOGV("magiqtouch", "Valid message received, length %d", msg_length);
      
      // Log message bytes for debugging
      if (esp_log_level_get("magiqtouch") >= ESP_LOG_VERBOSE) {
        char hex_str[256];
        int offset = 0;
        for (int i = 0; i < msg_length && offset < 250; i++) {
          offset += snprintf(hex_str + offset, sizeof(hex_str) - offset, "%02X ", msg_buffer[i]);
        }
        ESP_LOGV("magiqtouch", "Message data: %s", hex_str);
      }
      
      // Process control command messages only (no IoT module support)
      this->control_command_process(msg_buffer, msg_length);
      
      return true;
    } else {
      ESP_LOGW("magiqtouch", "CRC mismatch: got 0x%04X, expected 0x%04X", crc_raw, expected_crc);
    }
    
    return false;
  }
  
  void control_command_process(uint8_t *msg_buffer, int msg_length) {
    if (msg_length != 11) {
      return;
    }
    
    if (!this->check_pattern(msg_buffer, CONTROL_COMMAND_HEADER, sizeof(CONTROL_COMMAND_HEADER))) {
      return;
    }
    
    const uint8_t xx = msg_buffer[8];
    const uint8_t new_fan_speed = (uint8_t)(xx >> 4);
    const bool cooler_mode = (xx & 0x02) != 0;
    const uint8_t new_system_mode = cooler_mode ? 2 : 0;
    
    this->fan_speed_ = new_fan_speed;
    this->system_mode_ = new_system_mode;
    
    ESP_LOGD("magiqtouch", "Control command: fan=%d, cooler=%d", new_fan_speed, cooler_mode);
    
    // Update sensors
    this->update_sensors();
  }
  
  void update_drain_mode() {
    uint32_t current_time = millis();
    
    // Track when cooling mode ends
    if ((this->last_system_mode_ == 2 || this->last_system_mode_ == 3) && 
        this->system_mode_ != 2 && this->system_mode_ != 3) {
      // Just exited cooling mode
      this->last_cool_mode_end_time_ = current_time;
      ESP_LOGI("magiqtouch", "Cooling mode ended, drain timer started");
    }
    this->last_system_mode_ = this->system_mode_;
    
    // Check if drain mode should be automatically triggered
    if (this->last_cool_mode_end_time_ != 0 && !this->drain_mode_active_) {
      uint32_t time_since_cooling = current_time - this->last_cool_mode_end_time_;
      if (time_since_cooling >= COOL_TRANSITION_TO_DRAIN) {
        this->drain_mode_active_ = true;
        this->drain_mode_start_time_ = current_time;
        this->last_cool_mode_end_time_ = 0;  // Reset to prevent re-triggering
        ESP_LOGI("magiqtouch", "Auto-drain mode activated after 24-hour idle period");
      }
    }
    
    // Manual drain trigger
    if (this->drain_mode_manual_ && !this->drain_mode_active_) {
      this->drain_mode_active_ = true;
      this->drain_mode_start_time_ = current_time;
      this->drain_mode_manual_ = false;  // Reset manual flag
      ESP_LOGI("magiqtouch", "Manual drain mode activated");
    }
    
    // Check if drain mode should end
    if (this->drain_mode_active_) {
      uint32_t drain_elapsed = current_time - this->drain_mode_start_time_;
      if (drain_elapsed >= DRAIN_TIME) {
        this->drain_mode_active_ = false;
        ESP_LOGI("magiqtouch", "Drain mode completed");
      }
    }
    
    // Update drain mode sensor
    if (this->drain_mode_active_sensor_ != nullptr) {
      this->drain_mode_active_sensor_->publish_state(this->drain_mode_active_);
    }
  }
  
  void send_command_control(bool drain_active) {
    // Control command format: 02 10 00 31 00 01 02 00 XX YY YY
    // Where XX = (FanSpeed * 16) + modifiers
    
    const bool cooler_mode = (this->system_mode_ == 2) || (this->system_mode_ == 3);
    uint8_t effective_fan_speed = this->fan_speed_;
    
    uint8_t xx = (uint8_t)(effective_fan_speed << 4);
    if (cooler_mode || drain_active) {
      xx = (uint8_t)(xx + 2);
    }
    if (!this->system_power_) {
      xx = 0x00;  // Power off command
    }
    if (drain_active) {
      xx += 1;
    }
    
    uint8_t control_msg[] = { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00, xx };
    this->send_message(control_msg, 9, true);
  }
  
  void send_message(uint8_t *msg_buffer, int length, bool send_crc) {
    if (this->uart_ == nullptr) {
      ESP_LOGW("magiqtouch", "Cannot send message: UART not initialized");
      return;
    }
    
    // Calculate total bytes to send
    int total_bytes = length + (send_crc ? 2 : 0);
    
    // Log message being sent
    if (esp_log_level_get("magiqtouch") >= ESP_LOG_DEBUG) {
      char hex_str[256];
      int offset = 0;
      for (int i = 0; i < length && offset < 250; i++) {
        offset += snprintf(hex_str + offset, sizeof(hex_str) - offset, "%02X ", msg_buffer[i]);
      }
      if (send_crc) {
        uint16_t crc_val = this->modbus_crc(msg_buffer, length);
        offset += snprintf(hex_str + offset, sizeof(hex_str) - offset, "[CRC:%04X]", crc_val);
      }
      ESP_LOGD("magiqtouch", "Sending message (%d bytes): %s", total_bytes, hex_str);
    }
    
    // Enable RS485 transmitter
    if (this->rs485_en_pin_ != nullptr) {
      this->rs485_en_pin_->digital_write(true);
      // RS485 transceiver enable settling time (50us is within typical 5-50us range
      // for common transceivers like MAX485, MAX3485, SP485, etc.).
      // This blocking delay is necessary for proper RS485 timing.
      delayMicroseconds(50);
    }
    
    // Write message to UART
    for (int i = 0; i < length; i++) {
      this->uart_->write_byte(msg_buffer[i]);
    }
    
    // Add CRC if requested
    if (send_crc) {
      uint16_t crc_val = this->modbus_crc(msg_buffer, length);
      uint8_t high_byte = (crc_val >> 8) & 0xFF;
      uint8_t low_byte = crc_val & 0xFF;
      
      this->uart_->write_byte(high_byte);
      this->uart_->write_byte(low_byte);
    }
    
    // Flush UART
    this->uart_->flush();
    
    // RS485 guard time: ensure last byte fully transmits before disabling
    // At 9600 baud, 8N1: (total_bytes * 10 bits * 1000000 us) / 9600 baud
    // = ~1041.67 microseconds per byte, rounded up for safety
    uint32_t guard_time_us = (total_bytes * 10 * 1000000 + 9599) / 9600;
    delayMicroseconds(guard_time_us);
    
    // Disable RS485 transmitter (back to receive)
    if (this->rs485_en_pin_ != nullptr) {
      this->rs485_en_pin_->digital_write(false);
    }
  }
  
  uint16_t modbus_crc(uint8_t *buffer, int length) {
    uint8_t crc_hi = 0xFF;
    uint8_t crc_lo = 0xFF;
    
    for (int i = 0; i < length; i++) {
      int index = crc_lo ^ buffer[i];
      crc_lo = crc_hi ^ table_crc_hi[index];
      crc_hi = table_crc_lo[index];
    }
    
    return (crc_lo << 8) | crc_hi;
  }
  
  bool check_pattern(const uint8_t *buffer, const uint8_t *pattern, size_t length) {
    return memcmp(buffer, pattern, length) == 0;
  }
  
  void update_sensors() {
    // Update system mode text sensor
    if (this->system_mode_sensor_ != nullptr) {
      std::string mode_text = "Unknown";
      switch (this->system_mode_) {
        case 0: mode_text = "Fan External"; break;
        case 1: mode_text = "Fan Recycle"; break;
        case 2: mode_text = "Cooler Manual"; break;
        case 3: mode_text = "Cooler Auto"; break;
        case 4: mode_text = "Heater"; break;
      }
      this->system_mode_sensor_->publish_state(mode_text);
    }
  }
};

}  // namespace magiqtouch
}  // namespace esphome
