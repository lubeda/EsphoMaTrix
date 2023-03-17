from argparse import Namespace
import logging
import io
import requests

from esphome import core, automation
from esphome.components import display, font, time
import esphome.components.image as espImage
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_FILE, CONF_ID, CONF_BRIGHTNESS, CONF_RAW_DATA_ID,  CONF_TIME, CONF_DURATION, CONF_TRIGGER_ID
from esphome.core import CORE, HexInt
from esphome.cpp_generator import RawExpression
from .select import EHMTXSelect

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ["display", "light", "api"]
AUTO_LOAD = ["ehmtx"]
IMAGE_TYPE_RGB565 = 4
MAXFRAMES = 110
MAXICONS = 90
ICONWIDTH = 8
ICONHEIGHT = 8
ICONBUFFERSIZE = ICONWIDTH * ICONHEIGHT * 4
SVG_ICONSTART = '<svg width="80px" height="80px" viewBox="0 0 80 80">'
SVG_FULLSCREENSTART = '<svg width="320px" height="80px" viewBox="0 0 320 80">'

SVG_END = "</svg>"

def rgb565_svg(x,y,r,g,b):
    return f"<rect style=\"fill:rgb({(r << 3) | (r >> 2)},{(g << 2) | (g >> 4)},{(b << 3) | (b >> 2)});\" x=\"{x*10}\" y=\"{y*10}\" width=\"10\" height=\"10\"/>"

ehmtx_ns = cg.esphome_ns.namespace("esphome")
EHMTX_ = ehmtx_ns.class_("EHMTX", cg.Component)
Icons_ = ehmtx_ns.class_("EHMTX_Icon")

NextScreenTrigger = ehmtx_ns.class_(
    "EHMTXNextScreenTrigger", automation.Trigger.template(cg.std_string)
)

NextClockTrigger = ehmtx_ns.class_(
    "EHMTXNextClockTrigger", automation.Trigger.template(cg.std_string)
)

CONF_SHOWCLOCK = "show_clock"
CONF_CLOCK_INTERVAL = "clock_interval"
CONF_SHOWSCREEN = "show_screen"
CONF_EHMTX = "ehmtx"
CONF_URL = "url"
CONF_FLAG = "flag"
CONF_LAMEID = "lameid"
CONF_ICONS = "icons"
CONF_SHOWDOW = "dayofweek"
CONF_SHOWDATE = "show_date"
CONF_HOLD_TIME = "hold_time"
CONF_DISPLAY = "display8x32"
CONF_HTML = "html"
CONF_SCROLLINTERVALL = "scroll_intervall"
CONF_ANIMINTERVALL = "anim_intervall"
CONF_FONT_ID = "font_id"
CONF_YOFFSET = "yoffset"
CONF_XOFFSET = "xoffset"
CONF_PINGPONG = "pingpong"
CONF_TIME_FORMAT = "time_format"
CONF_DATE_FORMAT = "date_format"
CONF_SELECT = "ehmtxselect"
CONF_ON_NEXT_SCREEN = "on_next_screen"
CONF_ON_NEXT_CLOCK = "on_next_clock"
CONF_SHOW_SECONDS = "show_seconds"
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
        CONF_CLOCK_INTERVAL, default="60"
    ): cv.templatable(cv.positive_int),
    cv.Optional(
        CONF_SELECT, 
    ): cv.use_id(EHMTXSelect),
    cv.Optional(
        CONF_YOFFSET, default="6"
    ): cv.templatable(cv.int_range(min=-32, max=32)),
    cv.Optional(
        CONF_HTML, default=False
    ): cv.boolean,
    cv.Optional(
        CONF_SHOW_SECONDS, default=False
    ): cv.boolean,
    cv.Optional(
        CONF_SHOWDATE, default=True
    ): cv.boolean,
    cv.Optional(
        CONF_WEEK_ON_MONDAY, default=True
    ): cv.boolean,
    cv.Optional(
        CONF_SHOWDOW, default=True
    ): cv.boolean,
    cv.Optional(
        CONF_TIME_FORMAT, default="%H:%M"
    ): cv.string,
    cv.Optional(
        CONF_DATE_FORMAT, default="%d.%m."
    ): cv.string,
    cv.Optional(
        CONF_XOFFSET, default="1"
    ): cv.templatable(cv.int_range(min=-32, max=32)),
    cv.Optional(
        CONF_HOLD_TIME, default="2"
    ): cv.templatable(cv.int_range(min=0, max=3600)),
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
    cv.Optional(CONF_ON_NEXT_CLOCK): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(NextClockTrigger),
        }
    ),
    cv.Required(CONF_ICONS): cv.All(
        cv.ensure_list(
            {
                cv.Required(CONF_ID): cv.declare_id(Icons_),

                cv.Exclusive(CONF_FILE,"uri"): cv.file_,
                cv.Exclusive(CONF_URL,"uri"): cv.url,
                cv.Exclusive(CONF_LAMEID,"uri"): cv.string,
                cv.Optional(
                    CONF_DURATION, default="0"
                ): cv.templatable(cv.positive_int),
                cv.Optional(
                    CONF_PINGPONG, default=False
                ): cv.boolean,
                cv.GenerateID(CONF_RAW_DATA_ID): cv.declare_id(cg.uint8),
            }
        ),
        cv.Length(max=MAXICONS),
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
    template_ = await cg.templatable(config[CONF_BRIGHTNESS], args, cg.int_)
    cg.add(var.set_brightness(template_))

    return var

SET_COLOR_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_), 
        cv.Optional(CONF_RED,default=80): cv.templatable(cv.uint8_t,),
        cv.Optional(CONF_BLUE,default=80): cv.templatable(cv.uint8_t,),
        cv.Optional(CONF_GREEN,default=80): cv.templatable(cv.uint8_t,),
    }
)

SetClockColorAction = ehmtx_ns.class_("SetClockColor", automation.Action)

@automation.register_action(
    "ehmtx.clock.color", SetClockColorAction, SET_COLOR_ACTION_SCHEMA
)
async def ehmtx_set_clock_color_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))

    return var

SetTextColorAction = ehmtx_ns.class_("SetTextColor", automation.Action)

@automation.register_action(
    "ehmtx.text.color", SetTextColorAction, SET_COLOR_ACTION_SCHEMA
)
async def ehmtx_set_text_color_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))

    return var

SetAlarmColorAction = ehmtx_ns.class_("SetAlarmColor", automation.Action)

@automation.register_action(
    "ehmtx.alarm.color", SetAlarmColorAction, SET_COLOR_ACTION_SCHEMA
)
async def ehmtx_set_alarm_color_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))

    return var

SET_FLAG_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_), 
        cv.Optional(CONF_FLAG,default=True): cv.templatable(cv.boolean),
    }
)

SetShowDateAction = ehmtx_ns.class_("SetShowDate", automation.Action)

@automation.register_action(
    "ehmtx.show.date", SetShowDateAction, SET_FLAG_ACTION_SCHEMA
)
async def ehmtx_show_date_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_FLAG], args, cg.bool_)
    cg.add(var.set_flag(template_))
    
    return var

SetShowDayOfWeekAction = ehmtx_ns.class_("SetShowDayOfWeek", automation.Action)

@automation.register_action(
    "ehmtx.show.dayofweek", SetShowDayOfWeekAction, SET_FLAG_ACTION_SCHEMA
)

async def ehmtx_show_dayofweek_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_FLAG], args, cg.bool_)
    cg.add(var.set_flag(template_))
    
    return var

SetTodayColorAction = ehmtx_ns.class_("SetTodayColor", automation.Action)

@automation.register_action(
    "ehmtx.today.color", SetTodayColorAction, SET_COLOR_ACTION_SCHEMA
)
async def ehmtx_set_today_color_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))
    return var

SetWeekdayColorAction = ehmtx_ns.class_("SetWeekdayColor", automation.Action)

@automation.register_action(
    "ehmtx.weekday.color", SetWeekdayColorAction, SET_COLOR_ACTION_SCHEMA
)

async def ehmtx_set_week_color_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])

    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_RED], args, cg.int_)
    cg.add(var.set_red(template_))
    template_ = await cg.templatable(config[CONF_GREEN], args, cg.int_)
    cg.add(var.set_green(template_))
    template_ = await cg.templatable(config[CONF_BLUE], args, cg.int_)
    cg.add(var.set_blue(template_))

    return var

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

SetDisplayOnAction = ehmtx_ns.class_("SetDisplayOn", automation.Action)

DISPLAY_ON_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
    }
)

@automation.register_action(
    "ehmtx.display.on", SetDisplayOnAction, DISPLAY_ON_ACTION_SCHEMA
)
async def ehmtx_set_display_on_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    return var

SetDisplayOffAction = ehmtx_ns.class_("SetDisplayOff", automation.Action)

DISPLAY_OFF_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EHMTX_),
    }
)
@automation.register_action(
    "ehmtx.display.off", SetDisplayOffAction, DISPLAY_OFF_ACTION_SCHEMA
)
async def ehmtx_set_display_off_action_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)

    return var

CODEOWNERS = ["@lubeda"]

async def to_code(config):

    from PIL import Image, ImageSequence

    def openImageFile(path):
        try:
            return Image.open(path)
        except Exception as e:
            raise core.EsphomeError(f" ICONS: Could not load image file {path}: {e}")

    def thumbnails(frames):
        for frame in frames:
            thumbnail = frame.copy()
            thumbnail.thumbnail((32,8), Image.ANTIALIAS)
            yield thumbnail

    var = cg.new_Pvariable(config[CONF_ID])
    html_string = F"<HTML><HEAD><TITLE>{CORE.config_path}</TITLE></HEAD>"
    html_string += '''\
    <STYLE>
    </STYLE><BODY>\
'''
    for conf in config[CONF_ICONS]:
                
        if CONF_FILE in conf:
            path = CORE.relative_config_path(conf[CONF_FILE])
            try:
                image = openImageFile(path)
            except Exception as e:
                raise core.EsphomeError(f" ICONS: Could not load image file {path}: {e}")
        elif CONF_LAMEID in conf:
            r = requests.get("https://developer.lametric.com/content/apps/icon_thumbs/" + conf[CONF_LAMEID], timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download image file {conf[CONF_LAMEID]}: {conf[CONF_ID]}")
            image = Image.open(io.BytesIO(r.content))
        elif CONF_URL in conf:
            r = requests.get(conf[CONF_URL], timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download image file {conf[CONF_URL]}: {conf[CONF_ID]}")
            image = Image.open(io.BytesIO(r.content))
        
        width, height = image.size

        if hasattr(image, 'n_frames'):
            frames = min(image.n_frames, MAXFRAMES)
        else:
            frames = 1

        if ((width != 4*ICONWIDTH) or (width != ICONWIDTH)) and (height != ICONHEIGHT):
            logging.warning(f" icon wrong size valid 8x8 or 8x32: {conf[CONF_ID]} skipped!")
        else:
            if (conf[CONF_DURATION] == 0):
                try:
                    duration =  image.info['duration']         
                except:
                    duration = config[CONF_ANIMINTERVALL]
            else:
                duration = conf[CONF_DURATION]

            html_string += F"<BR><B>{conf[CONF_ID]}</B>&nbsp;-&nbsp;({duration} ms):<BR>"

            pos = 0 
            frameIndex = 0
            html_string += f"<DIV ID={conf[CONF_ID]}>"
            data = [0 for _ in range(ICONBUFFERSIZE * 2 * frames)]
            for frameIndex in range(frames):
                
                image.seek(frameIndex)
                frame = image.convert("RGB")
                pixels = list(frame.getdata())
                width, height = image.size
                if width == 8:  
                    html_string += SVG_ICONSTART
                else:
                    html_string += SVG_FULLSCREENSTART
                i = 0
                for pix in pixels:
                    R = pix[0] >> 3
                    G = pix[1] >> 2
                    B = pix[2] >> 3
                    x = (i % width)
                    y = i//width
                    i +=1
                    rgb = (R << 11) | (G << 5) | B
                    html_string += rgb565_svg(x,y,R,G,B)
                    data[pos] = rgb >> 8
                    pos += 1
                    data[pos] = rgb & 255
                    pos += 1
                html_string += SVG_END
            html_string += f"</DIV>"
        
            rhs = [HexInt(x) for x in data]

            prog_arr = cg.progmem_array(conf[CONF_RAW_DATA_ID], rhs)

            cg.new_Pvariable(
                conf[CONF_ID],
                prog_arr,
                width,
                height,
                frames,
                espImage.IMAGE_TYPE["RGB565"],
                str(conf[CONF_ID]),
                conf[CONF_PINGPONG],
                duration,
            )

            cg.add(var.add_icon(RawExpression(str(conf[CONF_ID]))))

    html_string += "</BODY></HTML>"
    
    if config[CONF_HTML]:
        try:
            with open(CORE.config_path.replace(".yaml","") + ".html", 'w') as f:
                f.truncate()
                f.write(html_string)
                f.close()
        except:
            print("Error writing HTML file")    
    
    cg.add(var.set_show_clock(config[CONF_SHOWCLOCK]))
    cg.add(var.set_clock_interval(config[CONF_CLOCK_INTERVAL]))
    cg.add(var.set_brightness(config[CONF_BRIGHTNESS]))
    cg.add(var.set_screen_time(config[CONF_SHOWSCREEN]))
    cg.add(var.set_duration(config[CONF_DURATION]))
    cg.add(var.set_scroll_intervall(config[CONF_SCROLLINTERVALL]))
    cg.add(var.set_anim_intervall(config[CONF_ANIMINTERVALL]))
    cg.add(var.set_week_start(config[CONF_WEEK_ON_MONDAY]))
    cg.add(var.set_time_format(config[CONF_TIME_FORMAT]))
    cg.add(var.set_date_format(config[CONF_DATE_FORMAT]))
    cg.add(var.set_show_day_of_week(config[CONF_SHOWDOW]))
    cg.add(var.set_hold_time(config[CONF_HOLD_TIME]))
    cg.add(var.set_show_date(config[CONF_SHOWDATE]))
    cg.add(var.set_show_seconds(config[CONF_SHOW_SECONDS]))
    cg.add(var.set_font_offset(config[CONF_XOFFSET], config[CONF_YOFFSET]))

    disp = await cg.get_variable(config[CONF_DISPLAY])
    cg.add(var.set_display(disp))

    f = await cg.get_variable(config[CONF_FONT_ID])
    cg.add(var.set_font(f))

    ehmtxtime = await cg.get_variable(config[CONF_TIME])
    cg.add(var.set_clock(ehmtxtime))

    if (config.get(CONF_SELECT)):
        ehmtxselect = await cg.get_variable(config[CONF_SELECT])
        cg.add(var.set_select(ehmtxselect))

    for conf in config.get(CONF_ON_NEXT_SCREEN, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.std_string, "x"), (cg.std_string, "y")], conf)

    for conf in config.get(CONF_ON_NEXT_CLOCK, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    await cg.register_component(var, config)
