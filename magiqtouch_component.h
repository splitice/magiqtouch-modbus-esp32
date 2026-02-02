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

// Message patterns
static const uint8_t IOT_MODULE_INFO_REQUEST[] = { 0xEB, 0x03, 0x03, 0xE4, 0x00, 0x05, 0xD3, 0x70 };
static const uint8_t IOT_MODULE_INFO_RESPONSE[] = { 0xEB, 0x03, 0x0A, 0x01, 0x09, 0x01, 0x09, 0x70, 0x90, 0x2C, 0x65, 0x27, 0x0B, 0xA8, 0x11 };
static const uint8_t IOT_MODULE_COMMAND_REQUEST[] = { 0xEB, 0x03, 0x0B, 0x0C, 0x00, 0x0D, 0x50, 0xE2 };
static const uint8_t IOT_MODULE_STATUS_REQUEST[] = { 0xEB, 0x03, 0x02, 0x58, 0x00, 0x12, 0x53, 0x66 };
static const uint8_t CONTROL_COMMAND_HEADER[] = { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00 };

// Pattern arrays for IoT module messages
static const uint8_t EB1005D80023[] = { 0xEB, 0x10, 0x05, 0xD8, 0x00, 0x23, 0x17, 0xED };
static const uint8_t EB1005FB0021[] = { 0xEB, 0x10, 0x05, 0xFB, 0x00, 0x21, 0x67, 0xE6 };
static const uint8_t EB10061C002C[] = { 0xEB, 0x10, 0x06, 0x1C, 0x00, 0x2C, 0x16, 0x50 };
static const uint8_t EB1006480004[] = { 0xEB, 0x10, 0x06, 0x48, 0x00, 0x04, 0x57, 0x9E };
static const uint8_t EB10064C0038[] = { 0xEB, 0x10, 0x06, 0x4C, 0x00, 0x38, 0x16, 0x4E };
static const uint8_t EB100684002A[] = { 0xEB, 0x10, 0x06, 0x84, 0x00, 0x2A, 0x17, 0xBD };
static const uint8_t EB1006AE002A[] = { 0xEB, 0x10, 0x06, 0xAE, 0x00, 0x2A, 0x36, 0x75 };
static const uint8_t EB1006D80019[] = { 0xEB, 0x10, 0x06, 0xD8, 0x00, 0x19, 0x97, 0xBA };
static const uint8_t EB1008E50001[] = { 0xEB, 0x10, 0x08, 0xE5, 0x00, 0x01, 0x04, 0x94 };
static const uint8_t EB1008E60032[] = { 0xEB, 0x10, 0x08, 0xE6, 0x00, 0x32, 0xB4, 0x81 };

class MagiqTouchClimate : public climate::Climate, public Component {
 public:
  MagiqTouchClimate() {}

  void setup() override {
    // Initialize state variables
    this->system_power_ = false;
    this->fan_speed_ = 5;
    this->system_mode_ = 0;
    this->target_temp_ = 20;
    this->target_temp2_ = 20;
    this->heater_zone1_ = false;
    this->heater_zone2_ = false;
    
    // Drain mode initialization
    this->drain_mode_manual_ = false;
    this->drain_mode_active_ = false;
    this->last_cool_mode_end_time_ = 0;
    this->drain_mode_start_time_ = 0;
    this->last_system_mode_ = 0;
    
    // Info variables
    this->command_info_ = 0;
    this->system_power_info_ = 0;
    this->evap_mode_info_ = 0;
    this->evap_fan_speed_info_ = 5;
    this->heater_mode_info_ = 0;
    this->heater_fan_speed_info_ = 0;
    this->zone1_enabled_info_ = 0;
    this->zone2_enabled_info_ = 0;
    this->zone1_temp_info_ = 0;
    this->zone2_temp_info_ = 0;
    this->target_temp_info_ = 0;
    this->target_temp2_info_ = 0;
    this->thermistor_temp_info_ = 0;
    this->automatic_clean_running_ = 0;
    
    // IoT command variables
    this->x2vl_ = 0x9F;
    this->x3vl_ = 0x21;
    this->x4vl_ = 0x11;  // System Off initially
    this->x5vl_ = 0x42;
    this->x6vl_ = 0x43;
    this->x7vl_ = 0x04;
    this->x8vl_ = 0x22;
    this->x10v_ = 0x24;
    this->x12v_ = 0x24;
    this->x14v_ = 0x00;
    this->x17v_ = 0x14;
    this->x18v_ = 0x14;
    
    this->iot_supported_ = false;
    this->send_command_ = false;
    
    // Serial buffer initialization
    this->serial_a_index_ = 0;
    this->serial_b_index_ = 0;
    this->s1_previous_millis_ = 0;
    this->s2_previous_millis_ = 0;
    
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
    this->relay_serial_panel();
    this->relay_serial_unit();
    
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

  void set_uart_panel(uart::UARTComponent *uart) { this->uart_panel_ = uart; }
  void set_uart_unit(uart::UARTComponent *uart) { this->uart_unit_ = uart; }
  void set_rs485_enable_pin(GPIOPin *pin) { this->rs485_en_pin_ = pin; }
  
  // Sensor setters
  void set_command_info_sensor(sensor::Sensor *sensor) { this->command_info_sensor_ = sensor; }
  void set_evap_fan_speed_sensor(sensor::Sensor *sensor) { this->evap_fan_speed_sensor_ = sensor; }
  void set_heater_fan_speed_sensor(sensor::Sensor *sensor) { this->heater_fan_speed_sensor_ = sensor; }
  void set_zone1_temp_sensor(sensor::Sensor *sensor) { this->zone1_temp_sensor_ = sensor; }
  void set_zone2_temp_sensor(sensor::Sensor *sensor) { this->zone2_temp_sensor_ = sensor; }
  void set_thermistor_temp_sensor(sensor::Sensor *sensor) { this->thermistor_temp_sensor_ = sensor; }
  void set_target_temp2_sensor(sensor::Sensor *sensor) { this->target_temp2_sensor_ = sensor; }
  
  // Binary sensor setters
  void set_zone1_enabled_sensor(binary_sensor::BinarySensor *sensor) { this->zone1_enabled_sensor_ = sensor; }
  void set_zone2_enabled_sensor(binary_sensor::BinarySensor *sensor) { this->zone2_enabled_sensor_ = sensor; }
  void set_drain_mode_active_sensor(binary_sensor::BinarySensor *sensor) { this->drain_mode_active_sensor_ = sensor; }
  void set_automatic_clean_sensor(binary_sensor::BinarySensor *sensor) { this->automatic_clean_sensor_ = sensor; }
  
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
          this->system_power_info_ = false;
          break;
        case climate::CLIMATE_MODE_FAN_ONLY:
          this->system_power_ = true;
          this->system_power_info_ = true;
          this->apply_system_mode(0);  // Fan - External
          break;
        case climate::CLIMATE_MODE_COOL:
          this->system_power_ = true;
          this->system_power_info_ = true;
          this->apply_system_mode(2);  // Cooler - Manual
          break;
        case climate::CLIMATE_MODE_HEAT:
          this->system_power_ = true;
          this->system_power_info_ = true;
          this->apply_system_mode(4);  // Heater
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
    
    // Trigger command send
    this->send_command_ = true;
    this->send_command_message();
    
    this->publish_state();
  }

  // Additional control methods
  void set_zone1(bool enabled) {
    this->heater_zone1_ = enabled;
    this->send_command_ = true;
    this->send_command_message();
  }
  
  void set_zone2(bool enabled) {
    this->heater_zone2_ = enabled;
    this->send_command_ = true;
    this->send_command_message();
  }
  
  void set_target_temp2(uint8_t temp) {
    if (temp >= 10 && temp <= 28) {
      this->target_temp2_ = temp;
      this->send_command_ = true;
      this->send_command_message();
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
  // UART components
  uart::UARTComponent *uart_panel_{nullptr};
  uart::UARTComponent *uart_unit_{nullptr};
  GPIOPin *rs485_en_pin_{nullptr};
  
  // Climate traits
  climate::ClimateTraits traits_;
  
  // Control variables
  bool system_power_;
  uint8_t fan_speed_;
  uint8_t system_mode_;
  uint8_t target_temp_;
  uint8_t target_temp2_;
  bool heater_zone1_;
  bool heater_zone2_;
  
  // Drain mode variables
  bool drain_mode_manual_;
  bool drain_mode_active_;
  uint32_t last_cool_mode_end_time_;
  uint32_t drain_mode_start_time_;
  uint8_t last_system_mode_;
  
  // Info variables
  uint16_t command_info_;
  uint8_t system_power_info_;
  uint8_t evap_mode_info_;
  uint8_t evap_fan_speed_info_;
  uint8_t heater_mode_info_;
  uint8_t heater_fan_speed_info_;
  uint8_t zone1_enabled_info_;
  uint8_t zone2_enabled_info_;
  uint8_t zone1_temp_info_;
  uint8_t zone2_temp_info_;
  uint8_t target_temp_info_;
  uint8_t target_temp2_info_;
  uint8_t thermistor_temp_info_;
  uint8_t automatic_clean_running_;
  
  // IoT command variables
  uint8_t x2vl_, x3vl_, x4vl_, x5vl_, x6vl_, x7vl_, x8vl_;
  uint8_t x10v_, x12v_, x14v_, x17v_, x18v_;
  bool iot_supported_;
  bool send_command_;
  
  // Serial buffers
  uint8_t serial_a_buffer_[MAXMSGSIZE];
  int serial_a_index_;
  uint32_t s1_previous_millis_;
  
  uint8_t serial_b_buffer_[MAXMSGSIZE];
  int serial_b_index_;
  uint32_t s2_previous_millis_;
  
  // Timing
  uint32_t last_command_send_{0};
  uint32_t last_drain_update_{0};
  
  // Sensors
  sensor::Sensor *command_info_sensor_{nullptr};
  sensor::Sensor *evap_fan_speed_sensor_{nullptr};
  sensor::Sensor *heater_fan_speed_sensor_{nullptr};
  sensor::Sensor *zone1_temp_sensor_{nullptr};
  sensor::Sensor *zone2_temp_sensor_{nullptr};
  sensor::Sensor *thermistor_temp_sensor_{nullptr};
  sensor::Sensor *target_temp2_sensor_{nullptr};
  
  binary_sensor::BinarySensor *zone1_enabled_sensor_{nullptr};
  binary_sensor::BinarySensor *zone2_enabled_sensor_{nullptr};
  binary_sensor::BinarySensor *drain_mode_active_sensor_{nullptr};
  binary_sensor::BinarySensor *automatic_clean_sensor_{nullptr};
  
  text_sensor::TextSensor *system_mode_sensor_{nullptr};

  // Helper methods
  
  uint16_t modbus_crc(uint8_t *buffer, uint16_t buffer_length) {
    uint8_t crc_hi = 0xFF;
    uint8_t crc_lo = 0xFF;
    unsigned int i;
    
    while (buffer_length--) {
      i = crc_hi ^ *buffer++;
      crc_hi = crc_lo ^ table_crc_hi[i];
      crc_lo = table_crc_lo[i];
    }
    
    return (crc_hi << 8 | crc_lo);
  }
  
  bool check_pattern(uint8_t *msg_buffer, const uint8_t *check_pattern, int check_length) {
    for (int i = 0; i < check_length; i++) {
      if (msg_buffer[i] != check_pattern[i]) {
        return false;
      }
    }
    return true;
  }
  
  bool check_pattern_confirm(uint8_t *msg_buffer, const uint8_t *check_pattern) {
    if (this->check_pattern(msg_buffer, check_pattern, 6)) {
      this->send_message(msg_buffer, 8, false);
      return true;
    }
    return false;
  }
  
  void apply_system_mode(uint8_t new_mode) {
    this->system_mode_ = new_mode;
    
    // Keep EvapModeInfo consistent with SystemMode (unless heat)
    if (new_mode == 4) {
      return;
    }
    
    uint8_t evap_mode_nibble;
    switch (new_mode) {
      case 0:  // Fan - External
        evap_mode_nibble = 0x09;
        break;
      case 2:  // Cooler - Manual
        evap_mode_nibble = 0x05;
        break;
      case 3:  // Cooler - Auto
        evap_mode_nibble = 0x07;
        break;
      default:
        return;
    }
    
    this->evap_mode_info_ = (this->evap_mode_info_ & 0xF0) | (evap_mode_nibble & 0x0F);
  }
  
  void relay_serial_panel() {
    if (this->uart_panel_ == nullptr) return;
    
    while (this->uart_panel_->available()) {
      uint8_t incoming_byte;
      this->uart_panel_->read_byte(&incoming_byte);
      
      uint32_t current_millis = millis();
      if (this->serial_a_index_ > 0 && current_millis - this->s1_previous_millis_ > GAPTHRESHOLD) {
        this->serial_a_index_ = 0;
      }
      this->s1_previous_millis_ = current_millis;
      
      if (this->serial_a_index_ == MAXMSGSIZE) {
        ESP_LOGW("magiqtouch", "Buffer limit reached on panel UART");
        this->serial_a_index_ = 0;
      }
      
      this->serial_a_buffer_[this->serial_a_index_++] = incoming_byte;
      
      if (this->process_message(this->serial_a_buffer_, this->serial_a_index_, 1)) {
        this->serial_a_index_ = 0;
      }
    }
  }
  
  void relay_serial_unit() {
    if (this->uart_unit_ == nullptr) return;
    
    while (this->uart_unit_->available()) {
      uint8_t incoming_byte;
      this->uart_unit_->read_byte(&incoming_byte);
      
      uint32_t current_millis = millis();
      if (this->serial_b_index_ > 0 && current_millis - this->s2_previous_millis_ > GAPTHRESHOLD) {
        this->serial_b_index_ = 0;
      }
      this->s2_previous_millis_ = current_millis;
      
      if (this->serial_b_index_ == MAXMSGSIZE) {
        ESP_LOGW("magiqtouch", "Buffer limit reached on unit UART");
        this->serial_b_index_ = 0;
      }
      
      this->serial_b_buffer_[this->serial_b_index_++] = incoming_byte;
      
      if (this->process_message(this->serial_b_buffer_, this->serial_b_index_, 2)) {
        this->serial_b_index_ = 0;
      }
    }
  }
  
  bool process_message(uint8_t *msg_buffer, int msg_length, int serial_id) {
    if (msg_length < 4) {
      return false;
    }
    
    // CRC validation
    uint16_t crc_raw = (msg_buffer[msg_length - 2] << 8 | msg_buffer[msg_length - 1]);
    uint16_t expected_crc = this->modbus_crc(msg_buffer, msg_length - 2);
    
    if (crc_raw == expected_crc) {
      ESP_LOGV("magiqtouch", "Valid message received on serial %d, length %d", serial_id, msg_length);
      
      // Process different message types
      this->control_command_process(msg_buffer, msg_length);
      this->iot_module_message_process(msg_buffer, msg_length);
      this->panel2_info(msg_buffer, msg_length);
      
      return true;
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
    this->evap_fan_speed_info_ = new_fan_speed;
    this->apply_system_mode(new_system_mode);
    
    ESP_LOGD("magiqtouch", "Control command: fan=%d, cooler=%d", new_fan_speed, cooler_mode);
  }
  
  void panel2_info(uint8_t *msg_buffer, int msg_length) {
    if (msg_length != 7) {
      return;
    }
    
    uint8_t panel_info_response[] = { 0x97, 0x03, 0x02 };
    if (this->check_pattern(msg_buffer, panel_info_response, 3)) {
      this->zone2_temp_info_ = msg_buffer[3];
      
      if (this->zone2_temp_sensor_ != nullptr) {
        this->zone2_temp_sensor_->publish_state(this->zone2_temp_info_);
      }
    }
  }
  
  void iot_module_message_process(uint8_t *msg_buffer, int msg_length) {
    if (msg_length < 8) {
      return;
    }
    
    // Check for pattern confirmations
    if (this->check_pattern_confirm(msg_buffer, EB1005D80023) ||
        this->check_pattern_confirm(msg_buffer, EB1005FB0021) ||
        this->check_pattern_confirm(msg_buffer, EB10061C002C) ||
        this->check_pattern_confirm(msg_buffer, EB1006480004) ||
        this->check_pattern_confirm(msg_buffer, EB10064C0038) ||
        this->check_pattern_confirm(msg_buffer, EB100684002A) ||
        this->check_pattern_confirm(msg_buffer, EB1006AE002A) ||
        this->check_pattern_confirm(msg_buffer, EB1006D80019)) {
      return;
    }
    
    // Power state message
    if (this->check_pattern_confirm(msg_buffer, EB1008E50001)) {
      uint16_t power_msg_val = (msg_buffer[7] << 8 | msg_buffer[8]);
      this->system_power_info_ = power_msg_val & 1;
      power_msg_val = power_msg_val >> 4;
      
      if (power_msg_val != this->command_info_) {
        ESP_LOGI("magiqtouch", "Settings change from control panel");
        
        this->command_info_ = power_msg_val;
        this->system_power_ = (this->system_power_info_ == 1);
        this->heater_zone1_ = (this->zone1_enabled_info_ == 1);
        this->heater_zone2_ = (this->zone2_enabled_info_ == 1);
        this->target_temp_ = this->target_temp_info_;
        this->target_temp2_ = this->target_temp2_info_;
        
        // Determine system mode
        uint8_t evap_mode_temp = this->evap_mode_info_ & 0x0F;
        if (evap_mode_temp == 0x01 || evap_mode_temp == 0x05) {
          this->system_mode_ = 2;  // Cooler Manual
        } else if (evap_mode_temp == 0x07) {
          this->system_mode_ = 3;  // Cooler Auto
        } else if (evap_mode_temp == 0x09) {
          this->system_mode_ = 0;  // Fan External
        } else if (this->heater_mode_info_ == 0x01) {
          this->system_mode_ = 4;  // Heater
        } else if (this->heater_mode_info_ == 0x03) {
          this->system_mode_ = 1;  // Fan Recycle
        }
        
        // Update fan speed
        if (this->heater_fan_speed_info_ > 0 && this->system_mode_ == 1) {
          this->fan_speed_ = this->heater_fan_speed_info_;
        } else if (this->system_mode_ == 0 || this->system_mode_ == 2) {
          this->fan_speed_ = this->evap_fan_speed_info_;
        }
        
        this->update_command_message();
        this->send_command_ = false;
        
        this->publish_sensors();
        this->publish_state();
      }
      return;
    }
    
    // Main info update message
    if (this->check_pattern_confirm(msg_buffer, EB1008E60032)) {
      if (msg_length == 109) {
        this->automatic_clean_running_ = msg_buffer[11];
        this->evap_mode_info_ = msg_buffer[12];
        this->evap_fan_speed_info_ = (msg_buffer[14] & 0x0F);
        this->heater_mode_info_ = msg_buffer[40];
        this->heater_fan_speed_info_ = (msg_buffer[42] & 0x0F);
        this->target_temp_info_ = msg_buffer[44];
        this->target_temp2_info_ = msg_buffer[90];
        this->thermistor_temp_info_ = msg_buffer[46];
        this->zone1_enabled_info_ = msg_buffer[80] & 1;
        this->zone2_enabled_info_ = (msg_buffer[80] >> 1) & 1;
        this->zone1_temp_info_ = msg_buffer[87];
        
        this->publish_sensors();
      }
      return;
    }
    
    // IoT module info request
    if (this->check_pattern(msg_buffer, IOT_MODULE_INFO_REQUEST, 8)) {
      this->send_message((uint8_t*)IOT_MODULE_INFO_RESPONSE, 15, false);
      return;
    }
    
    // IoT module status request
    if (this->check_pattern(msg_buffer, IOT_MODULE_STATUS_REQUEST, 8)) {
      this->send_iot_module_status();
      return;
    }
    
    // IoT module command request
    if (this->check_pattern(msg_buffer, IOT_MODULE_COMMAND_REQUEST, 8)) {
      this->send_command_message();
      return;
    }
  }
  
  void send_iot_module_status() {
    uint8_t wifi = 0x00;  // WiFi connected
    uint8_t iot_status_response[] = {
      0xEB, 0x03, 0x24, 0x00, wifi, 0x52, 0x4D, 0x32, 0x48,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x04, 0x00
    };
    this->send_message(iot_status_response, 39, true);
  }
  
  void update_command_message() {
    // Update power state
    if (this->x4vl_ == 0x11 && this->system_power_) {
      this->x4vl_ = 0x10;
      this->send_command_ = true;
    } else if (this->x4vl_ == 0x10 && !this->system_power_) {
      this->x4vl_ = 0x11;
      this->send_command_ = true;
    }
    
    uint8_t tmp_cfan_speed = 0x41 + (this->fan_speed_ * 2);
    uint8_t tmp_hfan_speed = 0x21 + (this->fan_speed_ * 2);
    
    if (this->system_mode_ == 0) {  // External Fan Mode
      this->set_xval(&this->x5vl_, 0x42);
      this->set_xval(&this->x6vl_, tmp_cfan_speed);
      this->set_xval(&this->x7vl_, 0x04);
      this->set_xval(&this->x8vl_, 0x22);
    } else if (this->system_mode_ == 1) {  // Recycle Fan Mode
      this->set_xval(&this->x5vl_, 0x02);
      this->set_xval(&this->x6vl_, tmp_cfan_speed - 1);
      this->set_xval(&this->x7vl_, 0x04);
      this->set_xval(&this->x8vl_, tmp_hfan_speed);
    } else if (this->system_mode_ == 2) {  // Cooler Mode Manual
      this->set_xval(&this->x5vl_, 0x02);
      this->set_xval(&this->x6vl_, tmp_cfan_speed);
      this->set_xval(&this->x7vl_, 0x04);
      this->set_xval(&this->x8vl_, 0x02);
    } else if (this->system_mode_ == 3) {  // Cooler Mode Auto
      uint16_t temp_val = 0x2003 + (this->target_temp_ * 0x20);
      uint8_t high_byte = (temp_val >> 8) & 0xFF;
      uint8_t low_byte = temp_val & 0xFF;
      
      this->set_xval(&this->x5vl_, high_byte);
      this->set_xval(&this->x6vl_, low_byte);
      this->set_xval(&this->x7vl_, 0x04);
      this->set_xval(&this->x8vl_, 0x02);
      
      uint8_t zone_temp = 0x2 * this->target_temp_;
      this->set_xval(&this->x10v_, zone_temp);
      this->set_xval(&this->x12v_, zone_temp);
      this->set_xval(&this->x17v_, 0);
      this->set_xval(&this->x18v_, this->target_temp_);
    } else if (this->system_mode_ == 4) {  // Heater Mode
      uint16_t temp_val = 0x02C3 + (0x40 * this->target_temp_);
      uint8_t high_byte = (temp_val >> 8) & 0xFF;
      uint8_t low_byte = temp_val & 0xFF;
      this->set_xval(&this->x7vl_, high_byte);
      this->set_xval(&this->x8vl_, low_byte);
      
      uint8_t zone_temp = 0x2 * this->target_temp_;
      this->set_xval(&this->x10v_, zone_temp);
      this->set_xval(&this->x12v_, zone_temp);
      
      uint8_t tmp_zone_val = 0x0;
      if (this->heater_zone1_) tmp_zone_val += 1;
      if (this->heater_zone2_) tmp_zone_val += 2;
      
      this->set_xval(&this->x14v_, tmp_zone_val);
      this->set_xval(&this->x17v_, this->target_temp2_);
      this->set_xval(&this->x18v_, this->target_temp_);
    }
  }
  
  void send_command_message() {
    if (!this->iot_supported_) {
      this->send_command_control(this->drain_mode_active_);
      return;
    }
    
    this->update_command_message();
    
    if (this->send_command_) {
      if (this->x2vl_ == 0xFF) {
        this->x2vl_ = 0x0;
      } else {
        this->x2vl_++;
      }
      this->send_command_ = false;
    }
    
    uint8_t iot_command_message[] = {
      0xEB, 0x03, 0x1A, 0x02, this->x2vl_, this->x3vl_, this->x4vl_, this->x5vl_,
      this->x6vl_, this->x7vl_, this->x8vl_, 0x00, this->x10v_, 0x00, this->x12v_, 0x00,
      this->x14v_, 0x00, 0x00, this->x17v_, this->x18v_, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00
    };
    this->send_message(iot_command_message, 29, true);
  }
  
  bool update_drain_mode() {
    uint32_t current_time = millis();
    
    // Track when cooling mode ends
    if ((this->last_system_mode_ == 2 || this->last_system_mode_ == 3) && 
        this->system_mode_ != 2 && this->system_mode_ != 3) {
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
        this->last_cool_mode_end_time_ = 0;
        ESP_LOGI("magiqtouch", "Auto-drain mode activated after 24-hour idle period");
        
        if (this->drain_mode_active_sensor_ != nullptr) {
          this->drain_mode_active_sensor_->publish_state(true);
        }
      }
    }
    
    // Manual drain trigger
    if (this->drain_mode_manual_ && !this->drain_mode_active_) {
      this->drain_mode_active_ = true;
      this->drain_mode_start_time_ = current_time;
      this->drain_mode_manual_ = false;
      ESP_LOGI("magiqtouch", "Manual drain mode activated");
      
      if (this->drain_mode_active_sensor_ != nullptr) {
        this->drain_mode_active_sensor_->publish_state(true);
      }
    }
    
    // Check if drain mode should end
    if (this->drain_mode_active_) {
      uint32_t drain_elapsed = current_time - this->drain_mode_start_time_;
      if (drain_elapsed >= DRAIN_TIME) {
        this->drain_mode_active_ = false;
        ESP_LOGI("magiqtouch", "Drain mode completed");
        
        if (this->drain_mode_active_sensor_ != nullptr) {
          this->drain_mode_active_sensor_->publish_state(false);
        }
      }
    }
    
    return this->drain_mode_active_;
  }
  
  void send_command_control(bool drain_active) {
    const bool cooler_mode = (this->system_mode_ == 2) || (this->system_mode_ == 3);
    uint8_t effective_fan_speed = this->fan_speed_;
    
    uint8_t xx = (uint8_t)(effective_fan_speed << 4);
    if (cooler_mode || drain_active) {
      xx = (uint8_t)(xx + 2);
    }
    if (!this->system_power_) {
      xx = 0x00;
    }
    if (drain_active) {
      xx += 1;
    }
    
    uint8_t control_msg[] = { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00, xx };
    this->send_message(control_msg, 9, true);
  }
  
  void set_xval(uint8_t *val, uint8_t new_val) {
    if (*val != new_val) {
      *val = new_val;
      this->send_command_ = true;
    }
  }
  
  void send_message(uint8_t *msg_buffer, int length, bool send_crc) {
    if (this->uart_panel_ == nullptr || this->uart_unit_ == nullptr) {
      return;
    }
    
    // Calculate total bytes to send
    int total_bytes = length + (send_crc ? 2 : 0);
    
    // Enable RS485 transmitter
    if (this->rs485_en_pin_ != nullptr) {
      this->rs485_en_pin_->digital_write(true);
      // RS485 transceiver enable settling time (50us is within typical 5-50us range
      // for common transceivers like MAX485, MAX3485, SP485, etc.)
      // This blocking delay is necessary for proper RS485 timing
      delayMicroseconds(50);
    }
    
    // Write message to both UARTs
    for (int i = 0; i < length; i++) {
      this->uart_panel_->write_byte(msg_buffer[i]);
      this->uart_unit_->write_byte(msg_buffer[i]);
    }
    
    // Add CRC if requested
    if (send_crc) {
      uint16_t crc_val = this->modbus_crc(msg_buffer, length);
      uint8_t high_byte = (crc_val >> 8) & 0xFF;
      uint8_t low_byte = crc_val & 0xFF;
      
      this->uart_panel_->write_byte(high_byte);
      this->uart_unit_->write_byte(high_byte);
      this->uart_panel_->write_byte(low_byte);
      this->uart_unit_->write_byte(low_byte);
    }
    
    // Flush UARTs
    this->uart_panel_->flush();
    this->uart_unit_->flush();
    
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
  
  void publish_sensors() {
    if (this->command_info_sensor_ != nullptr) {
      this->command_info_sensor_->publish_state(this->command_info_);
    }
    if (this->evap_fan_speed_sensor_ != nullptr) {
      this->evap_fan_speed_sensor_->publish_state(this->evap_fan_speed_info_);
    }
    if (this->heater_fan_speed_sensor_ != nullptr) {
      this->heater_fan_speed_sensor_->publish_state(this->heater_fan_speed_info_);
    }
    if (this->zone1_temp_sensor_ != nullptr) {
      this->zone1_temp_sensor_->publish_state(this->zone1_temp_info_);
    }
    if (this->zone2_temp_sensor_ != nullptr) {
      this->zone2_temp_sensor_->publish_state(this->zone2_temp_info_);
    }
    if (this->thermistor_temp_sensor_ != nullptr) {
      this->thermistor_temp_sensor_->publish_state(this->thermistor_temp_info_);
    }
    if (this->target_temp2_sensor_ != nullptr) {
      this->target_temp2_sensor_->publish_state(this->target_temp2_info_);
    }
    if (this->zone1_enabled_sensor_ != nullptr) {
      this->zone1_enabled_sensor_->publish_state(this->zone1_enabled_info_ == 1);
    }
    if (this->zone2_enabled_sensor_ != nullptr) {
      this->zone2_enabled_sensor_->publish_state(this->zone2_enabled_info_ == 1);
    }
    if (this->automatic_clean_sensor_ != nullptr) {
      this->automatic_clean_sensor_->publish_state(this->automatic_clean_running_ != 0);
    }
    if (this->system_mode_sensor_ != nullptr) {
      const char* mode_text = "Unknown";
      switch (this->system_mode_) {
        case 0: mode_text = "Fan External"; break;
        case 1: mode_text = "Fan Recycle"; break;
        case 2: mode_text = "Cooler Manual"; break;
        case 3: mode_text = "Cooler Auto"; break;
        case 4: mode_text = "Heater"; break;
      }
      this->system_mode_sensor_->publish_state(mode_text);
    }
    
    // Update climate current temperature
    if (this->thermistor_temp_info_ > 0) {
      this->current_temperature = this->thermistor_temp_info_;
    }
  }
};

}  // namespace magiqtouch
}  // namespace esphome
