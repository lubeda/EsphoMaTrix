import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import (
    CONF_ID,
)

CONF_EHMTX = "ehmtx"
select_ns = cg.esphome_ns.namespace("esphome")

EHMTXSelect = select_ns.class_(
    "EhmtxSelect", select.Select, cg.PollingComponent
)

CONFIG_SCHEMA = cv.All(
    select.SELECT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(EHMTXSelect),
        }
    ).extend(cv.polling_component_schema("30s")),
)

async def to_code(config):
    cg.add_define("USE_EHMTX_SELECT")
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await select.register_select(var, config, options=[])
