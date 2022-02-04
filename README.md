# EspHoMaTriX (ehmtx)
A simple DIY status display with an 8x32 RGB LED panel implemented sole with [esphome.io](https://esphome.io)

# Introduction

There are some "RGB-matrix" status displays out there, the commercial one from lamtric and some DIY alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/)
- [PixelIt](https://docs.bastelbunker.de/pixelit/)

The DIY solutions have their pros and cons, i am still using an awtrix. But the cons are so big (after my opinion) that i started an esphome.io variant with an optimized homeassistant integration. The main reason, for me is the homeassistant integration.

## State

**This is a somehow usable version!**

It is not as mature as awtrix and pixeltIt but it's doing what **i** need. I am not shure about the copyright of the font and the icons i use so this repo has only a sample icons included and the font has to be included like all fonts in esphome (see installation). In professional terms it is a beta version. From the structure of the source code it is a _chaos_ ;-) version. There will be possibly breaking changes in the upcomming code.

See it in action [youtube](https://www.youtube.com/watch?v=ZyaFj7ArIdY) (no sound but subtitles)

## Features

Based a on a 8x32 RGB matrix it displays a clock, the date and up to 16 other screens provided by home assistant. Each screen (value/text) can be associated with a 8x8 bit RGB icon or gif animation (see installation). The values/text can be updated or deleted from the display queue. Each screen has a lifetime, if not refreshed it will disapear.

# Installation

## Font
Download a small "pixel" TTF-font, i use ["monobit.ttf"](https://www.google.com/search?q=monobit.ttf). I modified this font with [FontForge](https://fontforge.org/) and added **€** on base of a **E**. Due to copyright i can't provide my modified version :-(.

```
font:
  - file: monobit.ttf
    id: EHMTX_font
    size: 16
    glyphs:  |
      !"%()+*=,-_.:°0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz€@
```

## icons/animations
Download and install all needed icons (.jpg/.png)/animations (.gif)under the "ehmtx"-key. All icons are automagically scaled to 8x8 on compile-time. 

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

All other solutions provide ready made icons, especialy lametric has a big database of [icons](https://developer.lametric.com/icons). Please check the copyright of the used icons you use. The amount of icons is limited to 64 in the code and also by the flashspace and the RAM of your board.

The index of the icons is the order of definition, in the above sample "temp" is 1 and garage is 2.

## esphome component

### More stable
At the moment it is more stable to use a local component to get this running. Copy the components subfolder to your esphome folder. If needed customize the yaml to your folder structure.

```
external_components:
   - source:
       type: local
       path: components # e.g. /config/esphome/components

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
  id: rgb328 # needed to reference the components in services etc.
  showclock: 6
  showscreen: 8
  display8x32: ehmtxdisplay
  time: ehmtxclock
  duration: 7
  font_id: ehmtxfont
  icons: 
    - file: sample.png  # use your icons/animations here
      id: boot 
    - file: sample.png
      id: temp 
    - file: sample.png
      id: garage
```

_Configuration variables:_
**id (Required, ID):** Manually specify the ID used for code generation and in service definitions.

**showclock (Optional, seconds):** duration to display the clock after this time the date is display until next "showscreen"

**showscreen (Optional, seconds):** duration to display a screen or a clock/date sequence 

**duration (Optional, minutes):** lifetime of a screen in minutes (default=5)

**yoffset (Optional, pixel):** yoffset of the font, default -5 (see installation/font)

**display8x32 (required, ID):** ID of the addressable display

**time (required, ID):** ID of the time component

## Usage without homeassistant

You can add screens locally and display data from any local sensor. See this sample

```
sensor:
  - platform: bh1750
    id: sensorlx
    ...
    on_value:
      then:
       lambda: |-
          char text[30];
          sprintf(text,"Light: %2.1f lx", id(sensorlx).state);
          id(rgb8x32)->add_screen_n("sun",  text );
```

Take care that the ```char text[30];``` has enough space to store the formated text. 

## Integration in homeassistant

Each device has to be integrated in homeassistant. It provides at least three services, all prefixed with the devicename e.g. "ehmtx".

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

Service **_brightness**

Sets the overall brightness of the display (0..255)

parameters:
- ```brightness```: from dark to bright (0..255)

Service **_alarm**

Sets an alarm, the alarm is like a normal screen but is displayed two minutes longer than a normal screen and has a red marker in the upper right corner.

parameters:
- ```icon```: The number of the predefined icons (see installation)
- ```text```: The text to be displayed

Service **_screen**

Queues a screen with an icon/animation and a text. Per icon there can only be one text. If you need to show e.g. an indoor and an outdoor temperature you have to use different icons (ids)!
You can update the text on the fly. If the screen is displayed and you change the text for the icon it will start a new lifetime (see ```duration```) with the new text. 

parameters:
- ```icon``` The number of the predefined icons (see installation)
- ```text``` The text to be displayed

Service **_screen_n**

Queues a screen with an icon and a text. As above but you can use the icon named instead the icon id. If the name is wrong the icon with the id 0 is choosen.

parameters:
- ```icon_name``` The name of the icons as in the yaml (see installation)
- ```text``` The text to be displayed

Service **del_screen_n**

Removes a screen from the display by icon name.

parameters:
- ```icon_name``` The name of the icons as in the yaml (see installation)

Service **_screen_t**

like above with a special duration. E.G. to indicate someones birthday you can use 24*60= 1440 minutes

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


Service **status**

This service displays the running queue and a list of icons with index

```
[13:10:10][I][EHMTX:175]: status status: 1  as: 1
[13:10:10][I][EHMTX:176]: status screen count: 3
[13:10:10][I][EHMTX:181]: status slot: 0 icon: 36  text: 47.9°C end: 400
[13:10:10][I][EHMTX:181]: status slot: 1 icon: 23  text: Supa langer Text end: 310
[13:10:10][I][EHMTX:181]: status slot: 2 icon: 1  text: 10.3°C end: 363
[13:10:10][I][EHMTX:186]: status icon: 0 name: boot
[13:10:10][I][EHMTX:186]: status icon: 1 name: temp
[13:10:10][I][EHMTX:186]: status icon: 2 name: garage
[13:10:10][I][EHMTX:186]: status icon: 3 name: wind
[13:10:10][I][EHMTX:186]: status icon: 4 name: rain
```

### use in automations

The easiest way to use ehmtx as a status display is to use the icon names as trigger id. In my example i have a icon named "wind" when the sensor.wind_speed has a new state this automation sends the new data to the screen with the icon named wind and so on.

```
alias: EHMTX 8266 Test
description: ''
trigger:
  - platform: numeric_state
    entity_id: sensor.wind_speed
    id: wind
  - platform: state
    entity_id: sensor.actual_temperature
    id: temp
  - platform: state
    entity_id: sensor.wg_cover_device
    id: cover
condition: []
action:
  - service: esphome.ehmtx8266_screen_n
    data:
      icon_name: '{{trigger.id}}'
      text: >-
        {{trigger.to_state.state}}{{trigger.to_state.attributes.unit_of_measurement}}
mode: queued
max: 10
```

## Hardware/Wifi

Adapt all other data in the yaml to your needs, I use GPIO04 as port for the display.


# Usage

The integration works with the homeassistant api so, after boot of the device, it take some seconds until the service calls work.

# Disclaimer
THE SOFTWARE IS PROVIDED "AS IS", use at your own risk!

