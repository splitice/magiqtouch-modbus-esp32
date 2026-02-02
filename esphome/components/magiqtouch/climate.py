import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart, sensor, binary_sensor, text_sensor
from esphome.const import CONF_ID
from esphome import pins
from . import magiqtouch_ns, MagiqTouchClimate

CONF_RS485_ENABLE_PIN = "rs485_enable_pin"
CONF_ZONE1_TEMP_SENSOR = "zone1_temp_sensor"
CONF_ZONE2_TEMP_SENSOR = "zone2_temp_sensor"
CONF_THERMISTOR_TEMP_SENSOR = "thermistor_temp_sensor"
CONF_ZONE1_ENABLED_SENSOR = "zone1_enabled_sensor"
CONF_ZONE2_ENABLED_SENSOR = "zone2_enabled_sensor"
CONF_DRAIN_MODE_ACTIVE_SENSOR = "drain_mode_active_sensor"
CONF_SYSTEM_MODE_SENSOR = "system_mode_sensor"

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(MagiqTouchClimate),
        cv.Optional(CONF_RS485_ENABLE_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_ZONE1_TEMP_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ZONE2_TEMP_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_THERMISTOR_TEMP_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_ZONE1_ENABLED_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_ZONE2_ENABLED_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_DRAIN_MODE_ACTIVE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_SYSTEM_MODE_SENSOR): cv.use_id(text_sensor.TextSensor),
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await uart.register_uart_device(var, config)

    if CONF_RS485_ENABLE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_RS485_ENABLE_PIN])
        cg.add(var.set_rs485_enable_pin(pin))

    if CONF_ZONE1_TEMP_SENSOR in config:
        sens = await cg.get_variable(config[CONF_ZONE1_TEMP_SENSOR])
        cg.add(var.set_zone1_temp_sensor(sens))

    if CONF_ZONE2_TEMP_SENSOR in config:
        sens = await cg.get_variable(config[CONF_ZONE2_TEMP_SENSOR])
        cg.add(var.set_zone2_temp_sensor(sens))

    if CONF_THERMISTOR_TEMP_SENSOR in config:
        sens = await cg.get_variable(config[CONF_THERMISTOR_TEMP_SENSOR])
        cg.add(var.set_thermistor_temp_sensor(sens))

    if CONF_ZONE1_ENABLED_SENSOR in config:
        sens = await cg.get_variable(config[CONF_ZONE1_ENABLED_SENSOR])
        cg.add(var.set_zone1_enabled_sensor(sens))

    if CONF_ZONE2_ENABLED_SENSOR in config:
        sens = await cg.get_variable(config[CONF_ZONE2_ENABLED_SENSOR])
        cg.add(var.set_zone2_enabled_sensor(sens))

    if CONF_DRAIN_MODE_ACTIVE_SENSOR in config:
        sens = await cg.get_variable(config[CONF_DRAIN_MODE_ACTIVE_SENSOR])
        cg.add(var.set_drain_mode_active_sensor(sens))

    if CONF_SYSTEM_MODE_SENSOR in config:
        sens = await cg.get_variable(config[CONF_SYSTEM_MODE_SENSOR])
        cg.add(var.set_system_mode_sensor(sens))
