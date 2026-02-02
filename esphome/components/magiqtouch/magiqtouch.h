#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace magiqtouch {

// Constants
static const uint8_t MAXMSGSIZE = 255;
static const uint8_t GAPTHRESHOLD = 50;
static const uint32_t DRAIN_TIME = 2 * 60 * 1000UL;  // 2 minutes
static const uint32_t COOL_TRANSITION_TO_DRAIN = 24 * 60 * 60 * 1000UL;  // 24 hours

// Control command header (no IoT module support)
static const uint8_t CONTROL_COMMAND_HEADER[] = { 0x02, 0x10, 0x00, 0x31, 0x00, 0x01, 0x02, 0x00 };

class MagiqTouchComponent : public Component, public uart::UARTDevice {
 public:
  MagiqTouchComponent() = default;

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_rs485_enable_pin(GPIOPin *pin) { this->rs485_en_pin_ = pin; }
  
  // Sensor setters
  void set_thermistor_temp_sensor(sensor::Sensor *sensor) { this->thermistor_temp_sensor_ = sensor; }
  
  // Binary sensor setters
  void set_drain_mode_active_sensor(binary_sensor::BinarySensor *sensor) { this->drain_mode_active_sensor_ = sensor; }
  
  // Text sensor setters
  void set_system_mode_sensor(text_sensor::TextSensor *sensor) { this->system_mode_sensor_ = sensor; }

  // Control methods (for buttons)
  void set_fan_speed(uint8_t speed);
  void set_power(bool power);
  void set_mode(uint8_t mode);
  void trigger_drain_mode();
  void cancel_drain_mode();

 protected:
  // RS485 enable pin
  GPIOPin *rs485_en_pin_{nullptr};
  
  // Control variables
  bool system_power_{false};
  uint8_t fan_speed_{5};
  uint8_t system_mode_{0};
  uint8_t target_temp_{20};
  
  // Drain mode variables
  bool drain_mode_manual_{false};
  bool drain_mode_active_{false};
  uint32_t last_cool_mode_end_time_{0};
  uint32_t drain_mode_start_time_{0};
  uint8_t last_system_mode_{0};
  
  // Timing variables
  uint32_t last_drain_update_{0};
  uint32_t last_command_send_{0};
  
  // Serial buffer
  uint8_t serial_buffer_[MAXMSGSIZE];
  int serial_index_{0};
  uint32_t previous_millis_{0};
  
  // Sensor pointers
  sensor::Sensor *thermistor_temp_sensor_{nullptr};
  
  // Binary sensor pointers
  binary_sensor::BinarySensor *drain_mode_active_sensor_{nullptr};
  
  // Text sensor pointers
  text_sensor::TextSensor *system_mode_sensor_{nullptr};

  // Helper methods
  void process_uart();
  bool process_message(uint8_t *msg_buffer, int msg_length);
  void control_command_process(uint8_t *msg_buffer, int msg_length);
  void update_drain_mode();
  void send_command_control(bool drain_active);
  void send_message(uint8_t *msg_buffer, int length, bool send_crc);
  uint16_t modbus_crc(uint8_t *buffer, int length);
  bool check_pattern(const uint8_t *buffer, const uint8_t *pattern, size_t length);
  void update_sensors();
};

}  // namespace magiqtouch
}  // namespace esphome
