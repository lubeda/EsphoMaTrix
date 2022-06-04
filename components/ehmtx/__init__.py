from argparse import Namespace
import logging
import io
import requests

from esphome import core, automation
from esphome.components import display, font, time
import esphome.components.image as espImage
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_FILE, CONF_ID, CONF_BRIGHTNESS, CONF_RAW_DATA_ID, CONF_TYPE, CONF_TIME, CONF_DURATION, CONF_TRIGGER_ID
from esphome.core import CORE, HexInt
from esphome.cpp_generator import RawExpression
from .select import EHMTXSelect

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ["display", "light", "api"]
AUTO_LOAD = ["ehmtx"]
MAXFRAMES = 20
MAXICONS=72
ISIZE = 8

ehmtx_ns = cg.esphome_ns.namespace("esphome")
EHMTX_ = ehmtx_ns.class_("EHMTX", cg.Component)
Icons_ = ehmtx_ns.class_("EHMTX_Icon")
# Triggers
NextScreenTrigger = ehmtx_ns.class_(
    "EHMTXNextScreenTrigger", automation.Trigger.template(cg.std_string)
)

CONF_SHOWCLOCK = "show_clock"
CONF_SHOWSCREEN = "show_screen"
CONF_EHMTX = "ehmtx"
CONF_URL = "url"
CONF_LAMEID = "lameid"
CONF_AWTRIXID = "awtrixid"
CONF_ICONS = "icons"
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
        CONF_SELECT, 
    ): cv.use_id(EHMTXSelect),
    cv.Optional(
        CONF_YOFFSET, default="6"
    ): cv.templatable(cv.int_range(min=-32, max=32)),
    cv.Optional(
        CONF_HTML, default=False
    ): cv.boolean,
    cv.Optional(
        CONF_WEEK_ON_MONDAY, default=True
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
                cv.Required(CONF_ID): cv.declare_id(Icons_),

                cv.Exclusive(CONF_FILE,"uri"): cv.file_,
                cv.Exclusive(CONF_URL,"uri"): cv.url,
                cv.Exclusive(CONF_LAMEID,"uri"): cv.string,
                cv.Exclusive(CONF_AWTRIXID,"uri"): cv.string,
                cv.Optional(
                    CONF_DURATION, default="0"
                ): cv.templatable(cv.positive_int),
                cv.Optional(
                    CONF_PINGPONG, default=False
                ): cv.boolean,
                cv.Optional(CONF_TYPE, default="RGB565"): cv.enum(
                    espImage.IMAGE_TYPE, upper=True
                ),
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
    template_ = await cg.templatable(config[CONF_BRIGHTNESS], args, cg.int32)
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

CODEOWNERS = ["@lubeda"]

async def to_code(config):

    from PIL import Image
    var = cg.new_Pvariable(config[CONF_ID])
    html_string = F"<HTML><HEAD><TITLE>{CORE.config_path}</TITLE></HEAD><STYLE> img {{ height: 40px; width: 40px; background: black;}} "
    body_string = ""
    for conf in config[CONF_ICONS]:
               
        html_string += F".{conf[CONF_ID]} {{ width: 40px; height: 40px; margin: 12px;  position: relative; font-size: 6px; }} "
        html_string += F".{conf[CONF_ID]}:after {{ content: \"\"; position: absolute; top: 0; left: 0;  width: 1em; height: 1em; box-shadow: "
  
        if CONF_FILE in conf:
            path = CORE.relative_config_path(conf[CONF_FILE])
            try:
                image = Image.open(path)
            except Exception as e:
                raise core.EsphomeError(f" ICONS: Could not load image file {path}: {e}")
        elif CONF_LAMEID in conf:
            r = requests.get("https://developer.lametric.com/content/apps/icon_thumbs/" + conf[CONF_LAMEID], timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download image file {conf[CONF_LAMEID]}: {conf[CONF_ID]}")
            image = Image.open(io.BytesIO(r.content))
        elif CONF_AWTRIXID in conf:
            r = requests.post("https://awtrix.blueforcer.de/icon",json={"reqType":"getIcon","ID":"" + conf[CONF_AWTRIXID] + ""}  , timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download awtrix data {conf[CONF_AWTRIXID]}: {conf[CONF_ID]}")
            awtrixdata = r.json()
            r = requests.get("https://awtrix.blueforcer.de/icons/"+conf[CONF_AWTRIXID], timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download awtrix icon {conf[CONF_URL]}: {conf[CONF_ID]}")
            image = Image.open(io.BytesIO(r.content))
            
        elif CONF_URL in conf:
            r = requests.get(conf[CONF_URL], timeout=4.0)
            if r.status_code != requests.codes.ok:
                raise core.EsphomeError(f" ICONS: Could not download image file {conf[CONF_URL]}: {conf[CONF_ID]}")
            image = Image.open(io.BytesIO(r.content))
        
        width, height = image.size
        if (width != ISIZE) or (height != ISIZE):
            image = image.resize([ISIZE, ISIZE])
            width, height = image.size

        if hasattr(image, 'n_frames'):
            frames = min(image.n_frames, MAXFRAMES)
        else:
            frames = 1
     
        # if CONF_FILE in conf:
        #     #body_string += str(conf[CONF_ID]) + ": <img src=\""+ conf[CONF_FILE] + "\" alt=\""+  str(conf[CONF_ID]) +"\">&nbsp;" 
        #     body_string += " "
        # elif CONF_URL in conf: 
        #     body_string += str(conf[CONF_ID]) + ": <img src=\""+ conf[CONF_URL] + "\" alt=\""+  str(conf[CONF_ID]) +"\">&nbsp;" 
        # elif CONF_LAMEID in conf: 
        #     body_string += str(conf[CONF_ID]) + ": <img src=\"https://developer.lametric.com/content/apps/icon_thumbs/"+ conf[CONF_LAMEID] + "\" alt=\""+  str(conf[CONF_ID]) +"\">&nbsp;"   
        
        if (conf[CONF_DURATION] == 0):
            try:
                duration =  image.info['duration']         
            except:
                duration = config[CONF_ANIMINTERVALL]
        else:
            duration = conf[CONF_DURATION]


        body_string += F"<B>{conf[CONF_ID]}</B>&nbsp;-&nbsp;({duration} ms):<DIV CLASS=\"{conf[CONF_ID]}\" align=left></DIV>"

        if conf[CONF_TYPE] == "GRAYSCALE":
            data = [0 for _ in range(ISIZE * ISIZE * frames)]
            pos = 0
            for frameIndex in range(frames):
                image.seek(frameIndex)
                frame = image.convert("L", dither=Image.NONE)
                pixels = list(frame.getdata())
                if len(pixels) != ISIZE * ISIZE:
                    raise core.EsphomeError(
                        f"Unexpected number of pixels in {path} frame {frameIndex}: ({len(pixels)} != {height*width})"
                    )
                for pix in pixels:
                    data[pos] = pix
                    pos += 1

        elif conf[CONF_TYPE] == "RGB24":
            data = [0 for _ in range(ISIZE * ISIZE * 3 * frames)]
            pos = 0
            for frameIndex in range(frames):
                image.seek(frameIndex)
                frame = image.convert("RGB")
                pixels = list(frame.getdata())
                if len(pixels) != ISIZE * ISIZE:
                    raise core.EsphomeError(
                        f"Unexpected number of pixels in {path} frame {frameIndex}: ({len(pixels)} != {height*width})"
                    )
                i = 0
                for pix in pixels:
                    x = 1+ (i % ISIZE)
                    y = i//ISIZE
                    i += 1
                    html_string += F"{x + (frameIndex*10)}em {y}em #{hex(pix[0]).replace('0x','').zfill(2)}{hex(pix[1]).replace('0x','').zfill(2)}{hex(pix[2]).replace('0x','').zfill(2)}, "
                    data[pos] = pix[0] & 248
                    pos += 1
                    data[pos] = pix[1] & 252
                    pos += 1
                    data[pos] = pix[2] & 248
                    pos += 1

        elif conf[CONF_TYPE] == "RGB565":
            
            pos = 0 
            frameIndex= 0
            if CONF_AWTRIXID in conf:
                if "data" in awtrixdata:
                    frames = len(awtrixdata["data"])
                    frameIndex =0
                    data = [0 for _ in range(ISIZE * ISIZE * 2 * frames)]
                    duration = awtrixdata["tick"]
                    for frame in awtrixdata["data"]:
                        frameIndex= +1
                        if len(frame) != ISIZE * ISIZE:
                            raise core.EsphomeError(
                                f"Unexpected number of pixels in awtrix"
                            )
                        i = 0
                        for pix in frame:
                            G = (pix & 0x07e0) >> 5
                            B = pix & 0x001f    
                            R = (pix & 0xF800) >> 8
                            x = 1+ (i % ISIZE)
                            y = i//ISIZE
                            i +=1
                            rgb = pix  # (R << 11) | (G << 5) | B
                            html_string += F"{x + (frameIndex*10)}em {y}em #{hex(R).replace('0x','').zfill(2)}{hex(G).replace('0x','').zfill(2)}{hex(B).replace('0x','').zfill(2)}, "
                            data[pos] = rgb >> 8
                            pos += 1               
                            data[pos] = rgb & 255
                            pos += 1

                else:
                    frames = 1
                    i = 0
                    data = [0 for _ in range(ISIZE * ISIZE * 2)]
                    for pix in awtrixdata:
                        x = 1+ (i % ISIZE)
                        y = i//ISIZE
                        i +=1
                        rgb = pix  
                        G = (pix & 0x07e0) >> 5
                        B = pix & 0x001f
                        R = (pix & 0xF800) >> 8
                        html_string += F"{x }em {y}em #{hex(R).replace('0x','').zfill(2)}{hex(G).replace('0x','').zfill(2)}{hex(B).replace('0x','').zfill(2)}, "
                        data[pos] = rgb >> 8
                        pos += 1               
                        data[pos] = rgb & 255
                        pos += 1              
            else:
                data = [0 for _ in range(ISIZE * ISIZE * 2 * frames)]
                for frameIndex in range(frames):
                    image.seek(frameIndex)
                    frame = image.convert("RGB")
                    pixels = list(frame.getdata())
                    if len(pixels) != ISIZE * ISIZE:
                        raise core.EsphomeError(
                            f"Unexpected number of pixels in {path} frame {frameIndex}: ({len(pixels)} != {height*width})"
                        )
                    i = 0
                    for pix in pixels:
                        R = pix[0] >> 3
                        G = pix[1] >> 2
                        B = pix[2] >> 3
                        x = 1+ (i % ISIZE)
                        y = i//ISIZE
                        i +=1
                        rgb = (R << 11) | (G << 5) | B
                        html_string += F"{x + (frameIndex*10)}em {y}em #{hex(pix[0]).replace('0x','').zfill(2)}{hex(pix[1]).replace('0x','').zfill(2)}{hex(pix[2]).replace('0x','').zfill(2)}, "
                        data[pos] = rgb >> 8
                        pos += 1
                        data[pos] = rgb & 255
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

        html_string =  html_string[:-2] + "; "
        rhs = [HexInt(x) for x in data]
        html_string += F"}} "

        prog_arr = cg.progmem_array(conf[CONF_RAW_DATA_ID], rhs)

        cg.new_Pvariable(
            conf[CONF_ID],
            prog_arr,
            width,
            height,
            frames,
            espImage.IMAGE_TYPE[conf[CONF_TYPE]],
            str(conf[CONF_ID]),
            conf[CONF_PINGPONG],
            duration,
        )

        cg.add(var.add_icon(RawExpression(str(conf[CONF_ID]))))
    html_string += "</STYLE><BODY>" + body_string
    html_string += "</BODY></HTML>"
    
    if config[CONF_HTML]:
        try:
            with open(CORE.config_path+".html", 'w') as f:
                f.truncate()
                f.write(html_string)
                f.close()
        except:
            print("Error writing HTML file")    
    
    cg.add(var.set_clock_time(config[CONF_SHOWCLOCK]))
    cg.add(var.set_default_brightness(config[CONF_BRIGHTNESS]))
    cg.add(var.set_screen_time(config[CONF_SHOWSCREEN]))
    cg.add(var.set_duration(config[CONF_DURATION]))
    cg.add(var.set_scroll_intervall(config[CONF_SCROLLINTERVALL]))
    cg.add(var.set_anim_intervall(config[CONF_ANIMINTERVALL]))
    cg.add(var.set_week_start(config[CONF_WEEK_ON_MONDAY]))
    cg.add(var.set_time_format(config[CONF_TIME_FORMAT]))
    cg.add(var.set_date_format(config[CONF_DATE_FORMAT]))
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

    await cg.register_component(var, config)
