from argparse import Namespace
import logging

from esphome import core
from esphome.components import display, font, time, text_sensor
import esphome.components.image as espImage
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_FILE, CONF_ID, CONF_BRIGHTNESS, CONF_RAW_DATA_ID, CONF_TYPE, CONF_TIME, CONF_DURATION
from esphome.core import CORE, HexInt
from esphome.cpp_generator import RawExpression

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ["display"]
AUTO_LOAD = ["ehmtx"]
MAXFRAMES=8

Icons_ = display.display_ns.class_("Animation")
EHMTX_ = cg.esphome_ns.namespace("EHMTX")

CONF_SHOWCLOCK = "show_clock"
CONF_SHOWSCREEN = "show_screen"
CONF_EHMTX = "ehmtx"
CONF_ICONS="icons"
CONF_DISPLAY="display8x32"
CONF_ICONID = "id"
CONF_SCROLLINTERVAL = "scroll_intervall"
CONF_ANIMINTERVAL = "anim_intervall"
CONF_FONT_ID = "font_id"
CONF_YOFFSET = "yoffset"
CONF_XOFFSET = "xoffset"


EHMTX_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(EHMTX_),
    cv.Required(CONF_TIME): cv.use_id(time),
    cv.Required(CONF_DISPLAY): cv.use_id(display),
    cv.Required(CONF_FONT_ID): cv.use_id(font),    
    cv.Optional(
            CONF_SHOWCLOCK, default="5"
            ): cv.templatable( cv.positive_int),
    cv.Optional(
            CONF_YOFFSET, default="-5"
            ): cv.templatable( cv.int_range(min=-32,max=32)),
    cv.Optional(
            CONF_XOFFSET, default="0"
            ): cv.templatable( cv.int_range(min=-32,max=32)),
    cv.Optional( CONF_SCROLLINTERVAL, default="80"
            ): cv.templatable( cv.positive_int),
    cv.Optional(
            CONF_ANIMINTERVAL, default="192"
            ): cv.templatable( cv.positive_int),
    cv.Optional(
                CONF_SHOWSCREEN, default="8"
            ): cv.templatable(cv.positive_int),
    cv.Optional(CONF_BRIGHTNESS, default=0.5): cv.percentage,
    cv.Optional(
                CONF_DURATION, default="5"
            ): cv.templatable(cv.positive_int),
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
        
        if hasattr( image, 'n_frames'):
            frames = min (image.n_frames,MAXFRAMES)
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
        
        cg.add(var.add_icon(RawExpression(str(conf[CONF_ID])+",\""+str(conf[CONF_ID])+"\"" ))) 

    cg.add(var.set_clock_time(config[CONF_SHOWCLOCK]))
    cg.add(var.set_brightness(config[CONF_BRIGHTNESS]))
    cg.add(var.set_screen_time(config[CONF_SHOWSCREEN]))
    cg.add(var.set_duration(config[CONF_DURATION]))
    cg.add(var.set_scroll_intervall(config[CONF_SCROLLINTERVAL]))
    cg.add(var.set_anim_intervall(config[CONF_ANIMINTERVAL]))
    cg.add(var.set_font_offset(config[CONF_XOFFSET],config[CONF_YOFFSET]))

    disp = await cg.get_variable(config[CONF_DISPLAY])
    cg.add(var.set_display(disp))

    f = await cg.get_variable(config[CONF_FONT_ID])
    cg.add(var.set_font(f))

    ehmtxtime = await cg.get_variable(config[CONF_TIME])
    cg.add(var.set_clock(ehmtxtime))

    # this should be part of the yaml configuration    
    
    



    
    