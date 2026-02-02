import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate

CODEOWNERS = ["@splitice"]

magiqtouch_ns = cg.esphome_ns.namespace("magiqtouch")
MagiqTouchClimate = magiqtouch_ns.class_("MagiqTouchClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = cv.Schema({})
