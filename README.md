# EsphoMaTrix
A simple DIY status display with an 8x32 RGB LED panel implemented with esphome.io

# Introdution

There are some status displays out there, the commercial one from lamtric and some DIY alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/)
- [PixelIt](https://docs.bastelbunker.de/pixelit/)

The DIY solutions have their pros and cons, i am still using an awtrix. But the cons are so big (after my opinion) that i started an esphome.io variant with an optimized homeassistant integration. The main reason, for me is the homeassistant integration.

## State

It is not as mature as awtrix and pixelt it but i does what i need. I am not shure about the copyright of the font and the icons i use so this repo has only a limited base of icons included and the font has to be includes like all fonts in esphome (see installation). In professional terms it is a beta version. From the structure of the source code it is a chaos version.

## Features

Based a on a 8x32 RGB matrix it displays a clock, the date and up to 16 other values provided by home assistant. Each value can be asssoiated with an 8x8 bit icon or gif animation (see installation). The values can be updated or deleted from the display queue.

## Integration in homeassistant

Each device has to be integrated in homeassistant. It provides three services, all prefixed with the devicename e.g. "emat".

Service **_alarm**

Sets an alarm, the alarm is like a normal screen but is displayed twice as long and has a red marker.

parameters:
'''icon''': The number of the predefined icons (see installation)
'''text''': The text to be displayed

Service **_screen**

Queues a screen with an icon and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature yopu have to use different icons!
If the screen is still displayed an you change the text for the icon it will start a new lifetime with the new text.

parameters:
'''icon''': The number of the predefined icons (see installation)
'''text''': The text to be displayed

Service **_del_screen**

removes the screen with the specified icon from the queue

parameters:
'''icon''': The number of the icons/screen to remove

# Installation

## Font
Download a "pixel" font, i use "monobit.ttf" for this font i need to define '''#define YFONTOFFSET -5''' in "EMaTcomponent.h" because of positioning on the matrix. The Font has to be named **EMaT_font**

## icons/animations
Download an install all needed icons as "animation", after that define an array with all needed icons. The name has to be EMaTicons!!!

'''display::Animation* EMaTicons[] = {startrek,wind,rain,call,door,temperature};'''

The icon with the index **0** is used for the boot screen. You can use gifs as animation and pngs as static "animations". All other solutions provide icon, especialy lametric has a big database of icons. Please check the copyright of the used icons. If needed scale to 8x8 pixel. The amount of icons is limited by the flashspace ans the RAM of your board.

## Hardware/Wifi

Adapt all other data in the yaml to your needs, I use GPIO04 as port for the display.




