# EspHoMaTriX (ehmtx)
A simple DIY status display with an 8x32 RGB LED panel implemented with esphome.io

# Introduction

There are some status displays out there, the commercial one from lamtric and some DIY alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/)
- [PixelIt](https://docs.bastelbunker.de/pixelit/)

The DIY solutions have their pros and cons, i am still using an awtrix. But the cons are so big (after my opinion) that i started an esphome.io variant with an optimized homeassistant integration. The main reason, for me is the homeassistant integration.

## State

**This is a somehow usable version!**

It is not as mature as awtrix and pixeltIt but it's doing what **i** need. I am not shure about the copyright of the font and the icons i use so this repo has only a sample icons included and the font has to be included like all fonts in esphome (see installation). In professional terms it is a beta version. From the structure of the source code it is a chaos version. There will be possibly breaking changes in the code.

See it in action [youtube](https://www.youtube.com/watch?v=ZyaFj7ArIdY)

## Features

Based a on a 8x32 RGB matrix it displays a clock, the date and up to 16 other screens provided by home assistant. Each screen (value/text) can be associated with a 8x8 bit RGB icon or gif animation (see installation). The values/text can be updated or deleted from the display queue. Each screen has a lifetime.

# Installation

## Font
Download a "pixel" font, i use "monobit.ttf".

```
font:
  - file: monobit.ttf
    id: EHMTX_font
    size: 16
    glyphs:  |
      !"%()+*=,-_.:°0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz€@
```

## icons/animations
Download and install all needed icons under the "ehmtx"-key. All icons are automagically scaled to 8x8 on compile-time. 

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

You can use gifs as animation and pngs as static "animations". Gif are limited to 8 frames to limit the flash space.

All other solutions provide icons, especialy lametric has a big database of icons. Please check the copyright of the used icons. The amount of icons is limited to 64 in the code and also by the flashspace and the RAM of your board.

The index of the icons is the order of definition, in the sample "temp" is 1 and garage is 2.

## components

### More stable
At the moment it is more stable to use a local component to get this running. Copy the components subfolder to your esphome folder. If needed customize the yaml to your folder structure.

```
external_components:
   - source:
       type: local
       path: components

```

### More features but perhaps breaking changes

Use the github repo as component.

```
external_components:
  - source:
      type: git
      url: https://github.com/lubeda/EsphoMaTrix
```

## YAML confugration

**Sample**
```
ehmtx:
  id: rgb328
  showclock: 6
  showscreen: 8
  display8x32: ehmtxdisplay
  time: ehmtxclock
  duration: 7
  font_id: ehmtxfont
  icons: 
    - file: sample.png
      id: boot 
    - file: sample.png
      id: temp 
    - file: sample.png
      id: garage
```

_Configuration variables:_
**id (Required, ID):** Manually specify the ID used for code generation and in service definitions.

**showclock (Optional, seconds):** duration to display the clock after this time the date is display until "showscreen"

**showscreen (Optional, seconds):** duration to display a screen or a clock/date sequence 

**duration (Optional, minutes):** lifetime of a screen in minutes (default=5)

**yoffset (Optional, pixel):** yoffset of the font, default -5 (see installation/font)

**display8x32 (required, ID):** ID of the addressable display

**time (required, ID):** ID of the time component

## Integration in homeassistant

Each device has to be integrated in homeassistant. It provides three services, all prefixed with the devicename e.g. "ehmtx".

### use the light component

To use the light component add the lambdas```on_turn_on``` and ```on_turn_off``` to the light

Sample:

```
light:
  - platform: neopixelbus
    id: ehmtx_light
    ....
    on_turn_on:
      lambda: |-
         id(rgb8x32)->set_enabled(false);
    on_turn_off:
       lambda: |-
         id(rgb8x32)->set_enabled(true);
```

### Services
All communication uses the api. The services are defined in the yaml. To define the services you need the id of the ehmtx-component e.g. ```id(rgb8x32)```.

*Sample*
```
api:
  services:
    - service: alarm
      variables:
        icon: int
        text: string
      then:
        lambda: |-
          id(rgb8x32)->add_alarm(icon,text);
```

Service **_alarm**

Sets an alarm, the alarm is like a normal screen but is displayed two minutes longer as a normal screen and has a red marker in the upper right corner.

parameters:
- ```icon```: The number of the predefined icons (see installation)
- ```text```: The text to be displayed

Service **_screen**

Queues a screen with an icon and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons (ids)!
If the screen is still displayed and you change the text for the icon it will start a new lifetime with the new text.

parameters:
- ```icon``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed

Service **_screen_n**

Queues a screen with an icon and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons (ids)!
If the screen is still displayed and you change the text for the icon it will start a new lifetime with the new text.

parameters:
- ```icon_name``` The name of the icons as in the yaml (see installation)
- ```text``` The text to be displayed

Service **del_screen_n**

Removes a screen from the display.

parameters:
- ```icon_name``` The name of the icons as in the yaml (see installation)

Service **_screen_t**

Queues a screen with an icon and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons (ids)!
If the screen is still displayed and you change the text for the icon it will start a new lifetime with the new text.

parameters:
- ```icon``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed
- ```duration``` The lifetime in minutes

Service **indicator_on**

Display a colored corner on all screen and the clock. You can define the color by parameter.

parameters:
- ```r``` red in 0..255
- ```g``` green in 0..255
- ```b``` blue in 0..255

Service **indicator_off**

removes the indicator


## Hardware/Wifi

Adapt all other data in the yaml to your needs, I use GPIO04 as port for the display.


# Usage

The integration works with the homeassistant api so, after boot of the device, it take some seconds until the service calls work.

# Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", use at your own risk!

