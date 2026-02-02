import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, binary_sensor, text_sensor
from esphome.const import CONF_ID
from esphome import pins

CODEOWNERS = ["@splitice"]
DEPENDENCIES = ["uart"]

magiqtouch_ns = cg.esphome_ns.namespace("magiqtouch")
MagiqTouchComponent = magiqtouch_ns.class_("MagiqTouchComponent", cg.Component, uart.UARTDevice)

CONF_RS485_ENABLE_PIN = "rs485_enable_pin"
CONF_DRAIN_MODE_ACTIVE_SENSOR = "drain_mode_active_sensor"
CONF_SYSTEM_MODE_SENSOR = "system_mode_sensor"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MagiqTouchComponent),
    cv.Optional(CONF_RS485_ENABLE_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_DRAIN_MODE_ACTIVE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional(CONF_SYSTEM_MODE_SENSOR): cv.use_id(text_sensor.TextSensor),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_RS485_ENABLE_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_RS485_ENABLE_PIN])
        cg.add(var.set_rs485_enable_pin(pin))

    if CONF_DRAIN_MODE_ACTIVE_SENSOR in config:
        sens = await cg.get_variable(config[CONF_DRAIN_MODE_ACTIVE_SENSOR])
        cg.add(var.set_drain_mode_active_sensor(sens))

    if CONF_SYSTEM_MODE_SENSOR in config:
        sens = await cg.get_variable(config[CONF_SYSTEM_MODE_SENSOR])
        cg.add(var.set_system_mode_sensor(sens))
