from argparse import Namespace
import logging

from esphome import core, automation
from esphome.components import display, font, time, text_sensor
import esphome.components.image as espImage
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_FILE, CONF_ID, CONF_BRIGHTNESS, CONF_RAW_DATA_ID, CONF_TYPE, CONF_TIME, CONF_DURATION, CONF_TRIGGER_ID
from esphome.core import CORE, HexInt
from esphome.cpp_generator import RawExpression

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ["display", "light", "api"]
AUTO_LOAD = ["ehmtx"]
MAXFRAMES = 8

Icons_ = display.display_ns.class_("Animation")
ehmtx_ns = cg.esphome_ns.namespace("esphome")
EHMTX_ = ehmtx_ns.class_("EHMTX", cg.Component)
# Triggers
NextScreenTrigger = ehmtx_ns.class_(
    "EHMTXNextScreenTrigger", automation.Trigger.template(cg.std_string)
)

CONF_SHOWCLOCK = "show_clock"
CONF_SHOWSCREEN = "show_screen"
CONF_EHMTX = "ehmtx"
CONF_ICONS = "icons"
CONF_DISPLAY = "display8x32"
CONF_ICONID = "id"
CONF_SCROLLINTERVALL = "scroll_intervall"
CONF_ANIMINTERVALL = "anim_intervall"
CONF_FONT_ID = "font_id"
CONF_YOFFSET = "yoffset"
CONF_XOFFSET = "xoffset"
CONF_ON_NEXT_SCREEN = "on_next_screen"
CONF_WEEK_ON_MONDAY = "week_start_monday"
CONF_ICON = "icon_name"
CONF_TEXT = "text"
CONF_ALARM = "alarm"

EHMTX_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(EHMTX_),
    cv.Required(CONF_TIME): cv.use_id(time),
    cv.Required(CONF_DISPLAY): cv.use_id(display),
    cv.Required(CONF_FONT_ID): cv.use_id(font),
    cv.Optional(
        CONF_SHOWCLOCK, default="5"
    ): cv.templatable(cv.positive_int),
    cv.Optional(
        CONF_YOFFSET, default="-5"
    ): cv.templatable(cv.int_range(min=-32, max=32)),
    cv.Optional(
        CONF_WEEK_ON_MONDAY, default=True
    ): cv.boolean,
    cv.Optional(
        CONF_XOFFSET, default="0"
    ): cv.templatable(cv.int_range(min=-32, max=32)),
    cv.Optional(CONF_SCROLLINTERVALL, default="80"
                ): cv.templatable(cv.positive_int),
    cv.Optional(
        CONF_ANIMINTERVALL, default="192"
    ): cv.templatable(cv.positive_int),
    cv.Optional(
        CONF_SHOWSCREEN, default="8"
    ): cv.templatable(cv.positive_int),
    cv.Optional(CONF_BRIGHTNESS, default=80): cv.templatable(cv.int_range(min=0, max=255)),
    cv.Optional(
        CONF_DURATION, default="5"
    ): cv.templatable(cv.positive_int),
    cv.Optional(CONF_ON_NEXT_SCREEN): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(NextScreenTrigger),
        }
    ),
    cv.Required(CONF_ICONS): cv.All(
        cv.ensure_list(
            {
                cv.Required(CONF_ICONID): cv.declare_id(Icons_),
                cv.Required(CONF_FILE): cv.file_,
                cv.Optional(CONF_TYPE, default="RGB24"): cv.enum(
                    espImage.IMAGE_TYPE, upper=True
                ),
                cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
            }
        ),
        cv.Length(max=64),
    )})

CONFIG_SCHEMA = cv.All(font.validate_pillow_installed, EHMTX_SCHEMA)

ADD_SCREEN_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
        cv.Required(CONF_ICON): cv.templatable(cv.string),
        cv.Required(CONF_TEXT): cv.templatable(cv.string),
        cv.Optional(CONF_DURATION): cv.templatable(cv.positive_int),
        cv.Optional(CONF_ALARM, default=False): cv.templatable(cv.boolean),
    }
)

AddScreenAction = ehmtx_ns.class_("AddScreenAction", automation.Action)

@automation.register_action(
    "ehmtx.add.screen", AddScreenAction, ADD_SCREEN_ACTION_SCHEMA
)
async def ehmtx_add_screen_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    template_ = await cg.templatable(config[CONF_ICON], args, cg.std_string)
    cg.add(var.set_icon(template_))

    template_ = await cg.templatable(config[CONF_TEXT], args, cg.std_string)
    cg.add(var.set_text(template_))

    if CONF_DURATION in config:
        template_ = await cg.templatable(config[CONF_DURATION], args, cg.uint8)
        cg.add(var.set_duration(template_))

    template_ = await cg.templatable(config[CONF_ALARM], args, bool)
    cg.add(var.set_alarm(template_))
    return var


SET_BRIGHTNESS_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
        cv.Optional(CONF_BRIGHTNESS, default=80): cv.templatable(cv.int_range(min=0, max=255)),
    }
)

SetBrightnessAction = ehmtx_ns.class_("SetBrightnessAction", automation.Action)


@automation.register_action(
    "ehmtx.set.brightness", SetBrightnessAction, SET_BRIGHTNESS_ACTION_SCHEMA
)
async def ehmtx_set_brightness_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_BRIGHTNESS], args, cg.int32)
    cg.add(var.set_brightness(template_))

    return var


SET_COLOR_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
        cv.Required(CONF_RED): cv.templatable(cv.uint8_t,),
        cv.Required(CONF_BLUE): cv.templatable(cv.uint8_t,),
        cv.Required(CONF_GREEN): cv.templatable(cv.uint8_t,),
    }
)

SetIndicatorOnAction = ehmtx_ns.class_("SetIndicatorOn", automation.Action)


@automation.register_action(
    "ehmtx.indicator.on", SetIndicatorOnAction, SET_COLOR_ACTION_SCHEMA
)
async def ehmtx_set_indicator_on_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))

    return var


DELETE_SCREEN_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
        cv.Required(CONF_ICON): cv.templatable(cv.string),
    }
)

DeleteScreenAction = ehmtx_ns.class_("DeleteScreen", automation.Action)


@automation.register_action(
    "ehmtx.delete.screen", DeleteScreenAction, DELETE_SCREEN_ACTION_SCHEMA
)
async def ehmtx_delete_screen_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_ICON], args, cg.std_string)
    cg.add(var.set_icon(template_))

    return var

ForceScreenAction = ehmtx_ns.class_("ForceScreen", automation.Action)


@automation.register_action(
    "ehmtx.force.screen", ForceScreenAction, DELETE_SCREEN_ACTION_SCHEMA
)
async def ehmtx_force_screen_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_ICON], args, cg.std_string)
    cg.add(var.set_icon(template_))

    return var


SetIndicatorOffAction = ehmtx_ns.class_("SetIndicatorOff", automation.Action)

INDICATOR_OFF_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
    }
)


@automation.register_action(
    "ehmtx.indicator.off", SetIndicatorOffAction, INDICATOR_OFF_ACTION_SCHEMA
)
async def ehmtx_set_indicator_off_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    return var


CODEOWNERS = ["@lubeda"]


async def to_code(config):

    from PIL import Image

    var = cg.new_Pvariable(config[CONF_ID])

    for conf in config[CONF_ICONS]:

        path = CORE.relative_config_path(conf[CONF_FILE])
        try:
            image = Image.open(path)
        except Exception as e:
            raise core.EsphomeError(f"Could not load image file {path}: {e}")

        width, height = image.size
        if (width != 8) or (height != 8):
            image = image.resize([8, 8])
            width, height = image.size

        if hasattr(image, 'n_frames'):
            frames = min(image.n_frames, MAXFRAMES)
        else:
            frames = 1

        if conf[CONF_TYPE] == "GRAYSCALE":
            data = [0 for _ in range(8 * 8 * frames)]
            pos = 0
            for frameIndex in range(frames):
                image.seek(frameIndex)
                frame = image.convert("L", dither=Image.NONE)
                pixels = list(frame.getdata())
                if len(pixels) != 8 * 8:
                    raise core.EsphomeError(
                        f"Unexpected number of pixels in {path} frame {frameIndex}: ({len(pixels)} != {height*width})"
                    )
                for pix in pixels:
                    data[pos] = pix
                    pos += 1

        elif conf[CONF_TYPE] == "RGB24":
            data = [0 for _ in range(8 * 8 * 3 * frames)]
            pos = 0
            for frameIndex in range(frames):
                image.seek(frameIndex)
                frame = image.convert("RGB")
                pixels = list(frame.getdata())
                if len(pixels) != 8 * 8:
                    raise core.EsphomeError(
                        f"Unexpected number of pixels in {path} frame {frameIndex}: ({len(pixels)} != {height*width})"
                    )
                for pix in pixels:
                    data[pos] = pix[0] & 248
                    pos += 1
                    data[pos] = pix[1] & 252
                    pos += 1
                    data[pos] = pix[2] & 248
                    pos += 1

        elif conf[CONF_TYPE] == "BINARY":
            width8 = ((width + 7) // 8) * 8
            data = [0 for _ in range((height * width8 // 8) * frames)]
            for frameIndex in range(frames):
                image.seek(frameIndex)
                frame = image.convert("1", dither=Image.NONE)
                for y in range(height):
                    for x in range(width):
                        if frame.getpixel((x, y)):
                            continue
                        pos = x + y * width8 + (height * width8 * frameIndex)
                        data[pos // 8] |= 0x80 >> (pos % 8)

        rhs = [HexInt(x) for x in data]
        prog_arr = cg.progmem_array(conf[CONF_RAW_DATA_ID], rhs)

        cg.new_Pvariable(
            conf[CONF_ID],
            prog_arr,
            width,
            height,
            frames,
            espImage.IMAGE_TYPE[conf[CONF_TYPE]],
        )

        cg.add(var.add_icon(RawExpression(
            str(conf[CONF_ID])+",\""+str(conf[CONF_ID])+"\"")))

    cg.add(var.set_clock_time(config[CONF_SHOWCLOCK]))
    cg.add(var.set_default_brightness(config[CONF_BRIGHTNESS]))
    cg.add(var.set_screen_time(config[CONF_SHOWSCREEN]))
    cg.add(var.set_duration(config[CONF_DURATION]))
    cg.add(var.set_scroll_intervall(config[CONF_SCROLLINTERVALL]))
    cg.add(var.set_anim_intervall(config[CONF_ANIMINTERVALL]))
    cg.add(var.set_week_start(config[CONF_WEEK_ON_MONDAY]))
    cg.add(var.set_font_offset(config[CONF_XOFFSET], config[CONF_YOFFSET]))

    disp = await cg.get_variable(config[CONF_DISPLAY])
    cg.add(var.set_display(disp))

    f = await cg.get_variable(config[CONF_FONT_ID])
    cg.add(var.set_font(f))

    ehmtxtime = await cg.get_variable(config[CONF_TIME])
    cg.add(var.set_clock(ehmtxtime))

    for conf in config.get(CONF_ON_NEXT_SCREEN, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "x"), (cg.std_string, "y")], conf)

    await cg.register_component(var, config)
