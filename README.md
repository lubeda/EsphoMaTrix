# EspHoMaTriX (ehmtx)
A simple DIY status display with an 8x32 RGB LED panel implemented with esphome.io

# Introdution

There are some status displays out there, the commercial one from lamtric and some DIY alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/)
- [PixelIt](https://docs.bastelbunker.de/pixelit/)

The DIY solutions have their pros and cons, i am still using an awtrix. But the cons are so big (after my opinion) that i started an esphome.io variant with an optimized homeassistant integration. The main reason, for me is the homeassistant integration.

## State

It is not as mature as awtrix and pixelt it but i does what **i** need. I am not shure about the copyright of the font and the icons i use so this repo has only a sample icons included and the font has to be included like all fonts in esphome (see installation). In professional terms it is a beta version. From the structure of the source code it is a chaos version.

## Features

Based a on a 8x32 RGB matrix it displays a clock, the date and up to 16 other values provided by home assistant. Each value/text can be associated with a 8x8 bit RGB icon or gif animation (see installation). The values/text can be updated or deleted from the display queue.

## Integration in homeassistant

Each device has to be integrated in homeassistant. It provides three services, all prefixed with the devicename e.g. "ehmtx".

Service **_alarm**

Sets an alarm, the alarm is like a normal screen but is displayed twice as long and has a red marker in the upper right corner.

parameters:
- ```icon```: The number of the predefined icons (see installation)
- ```text```: The text to be displayed

Service **_screen**

Queues a screen with an icon and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons!
If the screen is still displayed and you change the text for the icon it will start a new lifetime with the new text.

parameters:
- ```icon``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed

Service **_del_screen**

removes the screen with the specified icon from the queue

parameters:
- ```icon```: The number of the icons/screen to remove

# Installation

## Font
Download a "pixel" font, i use "monobit.ttf". For this font i need to define '''#define YFONTOFFSET -5''' in "EMaTcomponent.h" because of positioning on the matrix. The Font has to be named **EHMTX_font**

```
font:
  - file: monobit.ttf
    id: EHMTX_font
    size: 16
    glyphs:  |
      !"%()+*=,-_.:°0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz€@
```

## icons/animations
Download and install all needed icons under the "ehmtx"-key, after that define an array with all needed icons. The name has to be EMaTicons!!!

```
emhtx:
  icons: 
    - file: icons/rocket.gif
      id: boot 
    - file: temperature.png
      id: temp 
    - file: garage.gif
      id: garage
```

The icon with the index **0** is used for the boot screen. You can use gifs as animation and pngs as static "animations". 
All other solutions provide icon, especialy lametric has a big database of icons. Please check the copyright of the used icons. If needed scale to 8x8 pixel. The amount of icons is limited to 64 in the code and also by the flashspace and the RAM of your board.

The index of the icons is the order of definition, in the sample "temp" is 1 and garage is to. To review the icon order you can add an text-sensor to your config.

```
text_sensor:
  - platform: template
    name: "Icon list"
    lambda: |-
      return {EMaTiconlist};
    update_interval: 600s
```

## components and include

at the moment you need an include and a component to get this running. customize the yaml to your folder structure.

```
external_components:
   - source:
       type: local
       path: components

esphome:
  name: $devicename
  includes:
    - components/EHMTX_component.h
```

## Hardware/Wifi

Adapt all other data in the yaml to your needs, I use GPIO04 as port for the display.


# Usage

The integration works with the homeassistant api so, after boot of the device, it take some seconds until the service calls work.

# Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", use at your own risk!

