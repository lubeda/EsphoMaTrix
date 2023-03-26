# EspHoMaTriX (ehmtx)

A simple DIY status display, build with a flexible 8x32 RGB LED panel implemented with [esphome.io](https://esphome.io)

## Introduction

There are some "RGB-matrix" status displays/clocks out there, the commercial one from Lametric and some very good d.i.y.-alternatives. 

- [LaMetric](https://lametric.com/en-US/) commercial ~ 199€
- [Ulanzi TC001](https://www.aliexpress.com/item/1005005008682055.html) commercial ~ 50€
- [Awtrix](https://awtrixdocs.blueforcer.de/#/) (project has been discontinued after more than 4 years now in August 2022)
- [PixelIt](https://docs.bastelbunker.de/pixelit/) (project is under active development)

The other d.i.y. solutions have their pros and cons. I tried both and used AwTrix for a long time. But the cons are so big (in my opinion) that I started an esphome.io variant  targeted to an optimized Home Assistant integration. The main reason, for me is the Home Assistant integration!

There is a little hype around the Ulanzi TC001 pixel clock. This hardware can be used with **EspHoMaTriX** (with some limitations). You can connect the device and flash it via USB-C. As a starting point you can use the [``UlanziTC001.yaml``](https://github.com/lubeda/EsphoMaTrix/blob/main/UlanziTC001.yaml). Yet the LDR and battery sensor are not perfectly supported. For another use of the hardware see [PixelIT_Ulanzi](https://github.com/aptonline/PixelIt_Ulanzi) or [AWTRIX-LIGHT](https://github.com/Blueforcer/awtrix-light) firmwares.

See this German tutorial video with information on setting up your display [RGB-LED Status Display für Home Assistant mit ESPHome | ESPHoMaTrix](https://www.youtube.com/watch?v=DTd9vAhet9A).

Another german tutorial video focused at the Ulanzi [Smarte Pixel Clock über Home Assistant steuern - Entitäten / Icons und mehr in der Ulanzi](https://www.youtube.com/watch?v=LgaT0mNbl34)

See this [nice article](https://blakadder.com/esphome-pixel-clock/) about EsphoMaTrix on a Ulanzi TC001 from [blakadder](https://github.com/blakadder).

See this english discussions:
[Share your projects](https://community.home-assistant.io/t/esphomatrix-a-simple-clock-status-display/425325)
[ESPHOME](https://community.home-assistant.io/t/a-simple-diy-status-display-with-an-8x32-rgb-led/379051)

Or in german:
[Showroom](https://community.simon42.com/t/8x32-pixel-uhr-mit-homeassistant-anbindung/1076)

### State

It is a working solution with core functionality coded. Advanced features, like automatic brightness control can be done with esphome actions and automations.

See it in action on [YouTube](https://www.youtube.com/watch?v=ZyaFj7ArIdY) (no sound but subtitles).

### Features

Based a on a 8x32 RGB flexible matrix it displays a clock, the date and up to 16 other screens provided by Home Assistant. Each screen (value/text) can be associated with a 8x8 bit RGB icon or gif animation (see installation). The values/text can be updated or deleted from the display queue. Each screen has a lifetime, if not refreshed in its lifetime it will disappear.

You can use the [ehmtx32.yaml](https://github.com/lubeda/EsphoMaTrix/blob/main/ehmtx32.yaml) as sample for an ESP32. As mentioned you have to edit to your needs. So check font, icons, board and the GPIO pins for your display.

The file [ehmtx32.yaml](https://github.com/lubeda/EsphoMaTrix/blob/main/ehmtx32.yaml) uses the functions ehmtx provides, the sample file [ehmtx8266.yaml](https://github.com/lubeda/EsphoMaTrix/blob/main/ehmtx8266.yaml) uses actions where possible. You have to adapt the yaml to your hardware, since there are other RGB LED Displays possible.

## Installation

### Matrix Types
There are some different matrices-tapes on the market, to adapt them to **EspHoMaTriX** you have to find the proper pixelmapper. Here are the most common types for flexible 8x32 matrices:

#### Type 1 

under the display tag specify this pixelmapper:

```yaml
display:
  - platform: addressable_light
    .....
    pixel_mapper: |-
      if (x % 2 == 0) {
        return (x * 8) + y;
      }
      return (x * 8) + (7 - y);
    .....
```

#### Type 2 (e.g. Ulanzi TC001)

Under the display tag specify this pixelmapper:

```yaml
display:
  - platform: addressable_light
    .....
    pixel_mapper: |-
      if (y % 2 == 0) {
        return (y * 32) + x;
      }
      return (y * 32) + (31 - x);
    .....
```

### Font
Download a small "pixel" TTF-font, i use ["monobit.ttf"](https://www.google.com/search?q=monobit.ttf). You can modify this font with [FontForge](https://fontforge.org/) and added **€** on base of a **E** and so on. Due to copyright I can't provide my modified version :-(. Not all fonts are suitable for this minimalistic display. There are public domain fonts wich work well on the display e.g. [DMDSmall](https://www.pentacom.jp/pentacom/bitfontmaker2/gallery/?id=5993), details on alternative fonts are [here](https://blakadder.com/esphome-pixel-clock/#fonts).


```yaml
font:
  - file: monobit.ttf
    id: EHMTX_font
    size: 16
    glyphs:  |
      !"%()+*=,-_.:°0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz€@
```

### Icons and Animations

Download and install all needed icons (.jpg/.png) and animations (.gif) under the `ehmtx:` key. All icons have to be 8x8 or 8x32 piksels.

You can also specify an URL to directly download the image file. The URLs will only be downloaded once at compile time, so there is no additional traffic on the hosting website.

There are maximum 80 icons possible.

```yaml
emhtx:
  icons: 
    - id: boot 
      file: icons/rocket.gif
      duration: 75     
    - id: temp 
      file: temperature.png
    - id: yoga
      file: icons/yoga-bridge.gif
      pingpong: true
    - id: garage
      file: garage.gif
      duration: 100
    - id: homeassistant
      url: https://github.com/home-assistant/assets/raw/master/logo/logo-small.png      
```

The id of the icons is used later to configure the screens to display. So you should name them wisely.

First defined icon will be used as a fallback icon in case of an error, e.g. if you use a non-existing icon id.

GIFs are limited to 16 frames to limit the flash space. The first icon in your list is the fallback.

All other solutions provide ready made icons, especially Lametric has a big database of [icons](https://developer.lametric.com/icons). Please check the copyright of the icons you use. The amount of icons is limited to 80 in the code and also by the flash space and the RAM of your board.

***Parameters***

- **duration** (Optional, ms): in case of a gif file the component tries to read the default interval for each frame. The default/fallback interval is 192 ms. In case you need to override the default value set the duration per icon.
- **pingpong** (Optional, boolean): in case of a gif file you can reverse the frames instead of starting from the first.
- **file** (Exlusive, filename): a local filename
- **url** (Exclusive, url): an URL to download the icon

#### Icon preview helper

You can create a file with all icons and names as reference. The file is named like the yaml with the extension ".html" and places in esphome config directory.

```yaml
emhtx:
  html: true
```

Example result:

```html
<HTML><STYLE> img { height: 40px; width: 40px; background: black;}</STYLE><BODY>
error: <img src="_icons/error fatal.gif" alt="error">&nbsp;
leia: <img src="_icons/princess leia.gif" alt="leia">&nbsp;
</BODY></HTML>
```

#### Show all icons on your matrix

This code shows all icons once on boot up, depending on the amount of your icons it can take a while to see them all.

```yaml
esphome:
  ....
  on_boot:
    priority: -100
    # ...
    then:
      - lambda: !lambda |-
          id(rgb8x32)->show_all_icons();
```

### esphome custom component

#### local use

If you download the components-folder from the repo and install it in your esphome you have a stable installation. But if there are new features you won't see them. If needed customize the yaml to your folder structure.

```yaml
external_components:
   - source:
       type: local
       path: components # e.g. /config/esphome/components
```

#### use from repo direct

Use the github repo as component. Esphome refreshes the external components "only" once a day, perhaps you have to refresh it manually.

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/lubeda/EsphoMaTrix
```

## YAML configuration

_Example `ehmtx:` configuration_

```yaml
ehmtx:
  id: rgb8x32 # needed to reference the components in services and actions etc.
  clock_time: 6 
  screen_time: 8
  html: true
  display8x32: ehmtxdisplay
  time: EHMTX_clock
  font_id: EHMTX_font
  icons: 
    - file: sample.png  # use your icons/animations here
      id: boot 
    - file: celsius.png
      id: temp 
    - file: garage door.gif
      id: garage
```

_Configuration variables:_

**id** (Required, ID): Manually specify the ID used for code generation and in service definitions.

**clock_time** (Optional, seconds): duration to display the clock after this time the date is display until next "show_screen". If `show_date` is false `clock_time` is false and the clock will be display as long as a normal screen! Setting `clock_time` to 0 will not show the clock or date, if there are no screens the display will be blank until the next screen is sent.

You can set this value during runtime e.g. for a night mode

```
# sample for ulanzi tc001
binary_sensor:
  - platform: gpio
    pin:
      number: $left_button_pin
      inverted: true
    on_press:
      - logger.log: "Clock on"
      - lambda:
          id(rgb8x32)->set_clock_time(6);
    name: "clock on"
  - platform: gpio
    pin: 
      number: $right_button_pin
      inverted: true
    name: "Clock off"
    on_press:
      - logger.log: "clock off"
      - lambda:
          id(rgb8x32)->set_clock_time(0);
```

**clock_interval** (Optional, seconds): show the clock at least each x seconds, (default=60)

**show_screen** (Optional, seconds): duration to display a screen or a clock/date sequence, a long text will be scrolled at least two times 

![timing](./images/timing.png)

**hold_time** (Optional, seconds): extends the display time of the current screen in seconds (default=20)

**date_format** (Optional, string): formats the date display with [strftime syntax](https://esphome.io/components/time.html?highlight=strftime), defaults `"%d.%m."` (use `"%m.%d."` for the US)

**show_seconds** (Optional, boolean): toggle an indicator for seconds while the clock is displayed (default: false)) 

**time_format** (Optional, string): formats the date display with [strftime syntax](https://esphome.io/components/time.html?highlight=strftime), defaults `"%H:%M"` (use `"%I:%M%p"` for the US)

**yoffset** (Optional, pixel): yoffset the text is aligned BASELINE_LEFT, the baseline defaults to `6` 

**xoffset** (Optional, pixel): xoffset the text is aligned BASELINE_LEFT, the left defaults to `1`

**display8x32** (required, ID): ID of the addressable display

**dayofweek** (Optional, bool): draw the day indicator on the bottom of the screen. Disable e.g. if you want larger fonts, defaults to true. 

**show_date** (Optional, bool): if true, show the date for `screen_time - clock_time` seconds otherwise only shows the clock for `screen_time` seconds, defaults to true. 

**time** (required, ID): ID of the time component. The display shows `!t!` until the time source is valid.

**font** (required, ID): ID of the font component

**week_start_monday** (optional, bool): default Monday is first day of week, false => Sunday

**scroll_intervall** (Optional, ms): the interval in ms to scroll the text (default=80), should be a multiple of the ```update_interval``` from the display (default: 16ms)

**frame_intervall** (Optional, ms): the interval in ms to display the next animation/icon frame (default = 192), should be a multiple of the ```update_interval``` from the display (default = 16)

**html** (Optional, boolean): If true generate the html (_filename_.yaml.html) file to show all included icons.  (default = `false`)

### Usage without Home Assistant

You can add screens locally and display data directly from any local sensor. See this sample:

```yaml
sensor:
  - platform: bh1750
    id: sensorlx
    ...
    on_value:
      then:
        lambda: |-
          char text[30];
          sprintf(text,"Light: %2.1f lx", id(sensorlx).state);
           id(rgb8x32)->add_screen("sun", text, 5,11, false); // 5 Minutes,each time at least 11 seconds  no alarm
```

Take care that the ```char text[30];``` has enough space to store the formated text. 

### Local trigger

#### on_next_screen
There is a trigger available to do some local magic. The trigger ```on_next_screen``` is triggered every time a new screen is displayed (it doesn't trigger on the clock/date display!!). In lambda's you can use two local string variables:

**x** (Name of the icon, std::string): value to use in lambda

**y** (displayed text, std::string): value to use in lambda

See the examples:

##### Write information to homeassistant log

```yaml
ehmtx:
  ....
  on_next_screen:
    lambda: |-
        ESP_LOGD("TriggerTest","Iconname: %s",x.c_str());
        ESP_LOGI("TriggerTest","Text: %s",y.c_str());
```

##### Change the text color like crazy

```yaml
ehmtx:
  ....
  on_next_screen:
    lambda: |-
      id(rgb8x32)->set_text_color(rand() % 255, rand() % 255, rand() % 255);
```

##### Send an event to Home Assistant

To send data back to home assistant you can use events.

```yaml
ehmtx:
  ....
  on_next_screen:
    - homeassistant.event:
      event: esphome.next_screen
      data_template:
          iconname: !lambda "return x.c_str();"
          text: !lambda "return y.c_str();"
```

#### on_next_clock
The trigger ```on_next_clock``` is triggered every time a new clock display circle starts.
See the examples:

##### Change Clock colors like crazy for each clock circle

```yaml
ehmtx:
  ....
  on_next_clock:
    lambda: |-
      id(rgb8x32)->set_clock_color(rand() % 255, rand() % 255, rand() % 255);
      id(rgb8x32)->set_weekday_color(rand() % 255, rand() % 255, rand() % 255);
      id(rgb8x32)->set_today_color(rand() % 255, rand() % 255, rand() % 255);
```


### Actions

For local automations you can use actions. This is the normal way of automations. The ```id(rgb8x32)->``` style will also work.

##### Show date

You can dynamically enable or disable the display of the date see parameter `show_date`.

```yaml
    - ehmtx.show.date:
        id: rgb8x32
        flag: !lambda return true;
```

##### Show day of week

You can dynamically enable or disable the display of the day of week, see parameter `day_of_week`.

```yaml
    - ehmtx.show.dayofweek:
        id: rgb8x32
        flag: !lambda return true;
```

##### Force screen

Force the selected screen ```icon_name``` to be displayed next. Afterwards the loop is continuing from this screen. e.g. helpfull for alarms. Or after an update of the value/text.

```yaml
    - ehmtx.force.screen:
        id: rgb8x32
        icon_name: !lambda return icon_name;
```

##### Set (alarm/clock/gauge/text/today/weekday) color action

Sets the color of the selected element

You have to use use id of your ehmtx component, e.g. `rgb8x32`

```yaml
     - ehmtx.***.color:
        id: rgb8x32
        red: !lambda return r;
        green: !lambda return g;
        blue: !lambda return b;
```

valid elements:

- `ehmtx.alarm.color:`
- `ehmtx.clock.color:`
- `ehmtx.gauge.color:`
- `ehmtx.text.color:`
- `ehmtx.today.color:`
- `ehmtx.weekday.color:`
- ```red, green, blue```: the color components (`0..255`) _(default = `80`)_

*Example*

```yaml
esphome:
  name: $devicename
  on_boot:
    priority: -100
    then: 
      - ehmtx.text.color:
          id: rgb8x32
          red: !lambda return 200;
          blue: !lambda return 170;
      - ehmtx.today.color:
          id: rgb8x32
          red: !lambda return 10;
          green: !lambda return 250;
      - ehmtx.clock.color:
          id: rgb8x32
          red: !lambda return 50;
          green: !lambda return 150;
          blue: !lambda return 230;
      - ehmtx.weekday.color:
          id: rgb8x32
          red: !lambda return 250;
          green: !lambda return 50;
          blue: !lambda return 30;
```

##### Add screen to loop

```yaml
        - ehmtx.add.screen:
            id: rgb8x32
            text: !lambda return text;
            icon_name: !lambda return icon_name;
            lifetime: 7
            alarm: false
```

parameters:

- **id** (required, ID): ID of the ehmtx component
- **text** (required, string): the text to display
- **icon_name** (required, string): the name of the icon to display
- **lifetime** (optional, int): the lifetime of the screen in minutes (default=5)
- **alarm** (optional, bool): if alarm set true (default = false)

## Hardware/Wi-Fi

Adapt all other data in the yaml to your needs, I use GPIO04/GPIO16 (esp8266/ESP32) as port for the display.

## Integration in Home Assistant

To control your display it has to be integrated in Home Assistant. Then it provides a number of services, all prefixed with the configured `devicename` e.g. "ehmtx". See the default services marked as **(D)** [below](https://github.com/lubeda/EsphoMaTrix#services), but you can add your own.

### Use the light component

To use the light component add the sample lambdas```on_turn_on``` and ```on_turn_off``` to the light component.

*Example*

```yaml
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

All communication with Home Assistant use the homeasistant-api. The services can be provided by default or also defined additionally in the yaml. To define the additional services you need the id of the ehmtx-component e.g. ```id(rgb8x32)```.

*Example*

```yaml
api:
  services:
    - service: alarm
      variables:
        icon_name: string
        text: string
      then:
        lambda: |-
          id(rgb8x32)->add_screen(icon_name, text, 7,20, true); // 7 minutes lifetime/screen time 20 sec/alarm=true
```

**(D)** Service **brightness**

Sets the overall brightness of the display (`0..255`)

_parameters:_

- ```brightness```: from dark to bright (`0..255`) (default = `80`) as set in the light component by ```color_correct: [30%, 30%, 30%]```

There's an easier way by using a number component:

```yaml
number:
  - platform: template
    name: "LED brightness"
    min_value: 0
    max_value: 255
    step: 1
    lambda: |-
      return id(rgb8x32)->get_brightness();
    set_action:
      lambda: |-
        id(rgb8x32)->set_brightness(x);
```

Service **screen**

Queues a screen with an icon/animation and a text. There can only be one text per icon id. If you need to show e.g. an indoor and an outdoor temperature you have to use different icon id's!

You can update the text on the fly. If the screen is displayed and you change the text for the icon, it will start a new lifetime (see ```lifetime```) with the new text.

_parameters:_

- ```icon_name```: The number of the predefined icons (see installation)
- ```text```: The text to be displayed

Service **alarm**

Alarm is like a regular screen but it is displayed two minutes longer and has a red text color and a red marker in the upper right corner.

_parameters:_

- ```icon_name```: The name of the predefined icon id (see installation)
- ```text```: The text to be displayed

**(D)** Service **del_screen**

Removes a screen from the display by icon name. If this screen is actually display while sending this command the screen will be displayed until its "show_screen"-time has ended.

Optionally you can suffix a * to the icon name to perform a wildcard delete which will delete all screens beginning with the icon_name specified.

For example if you have multiple icons named weather_sunny, weather_rain & weather_cloudy, you can issue a del_screen weather_* to remove whichever screen is currently in a slot and replace it with a new weather screen.

_parameters:_

- ```icon_name```: Icon `id` defined in the yaml (see installation)

**(D)** Service **indicator_on** / **indicator_off**

Turns indicator on/off

Display a colored corner on all screens and the clock. You can define the color by parameter.

_parameters:_

- ```r``` red in 0..255
- ```g``` green in 0..255
- ```b``` blue in 0..255

**(D)** Service **alarm_color** / **clock_color** / **gauge_color** / **text_color** / **today_color** / **weekday_color**

Set the color of the named text-type

_parameters:_

- ```r``` red in 0..255
- ```g``` green in 0..255
- ```b``` blue in 0..255

**(D)** Service **display_on** / **display_off**

Turns the display on or off

There's an easier way in using a switch component:

```yaml
switch:
  - platform: template
    name: "$devicename Display"
    icon: "mdi:power"
    restore_mode: ALWAYS_ON
    lambda: |-
      return id(rgb8x32)->show_display;
    turn_on_action:
      lambda: |-
        id(rgb8x32)->set_display_on();
    turn_off_action:
      lambda: |-
        id(rgb8x32)->set_display_off();
```

Service **skip_screen**

If there is more than one screen in the queue, skip to the next screen.

e.g. on the Ulanzi TC001

```yaml
binary_sensor:
  - platform: gpio
    pin:
      number: $left_button_pin
      inverted: true
    on_press:
      lambda:
        id(rgb8x32)->skip_screen();
```

Service **hold_screen**

Displays the current screen for configured ammount (see **hold_time**) (default=20) seconds longer.

e.g. on the Ulanzi TC001

```
binary_sensor:
  - platform: gpio
    pin:
      number: $right_button_pin
      inverted: true
    on_press:
      lambda:
        id(rgb8x32)->hold_screen();
```


**(D)** Service **status**

This service displays the running queue and a list of icons in the logs

```log
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

**(D)** Service **show_all_icons**

Display all of your icons sequentially by ID.

Service **gauge_value** / **gauge_off**

**(D)** Turns gauge on/off
Displays a colored gauge. You can define the color by parameter.

_parameters:_

- ```percent``` gauge percentage

### Use in Home Assistant automations

The easiest way to use ehmtx as a status display is to use the icon names as trigger id. In my example i have an icon named "wind" when the sensor.wind_speed has a new state this automation sends the new data to the screen with the icon named "wind" and so on.

```yaml
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
  - service: esphome.ehmtx8266_screen
    data:
      icon_name: '{{trigger.id}}'
      text: >-
        {{trigger.to_state.state}}{{trigger.to_state.attributes.unit_of_measurement}}
mode: queued
max: 10
```

#### Display precision after home assistant 2023.3.0

See [templating](https://www.home-assistant.io/docs/configuration/templating/#states) for possibilities to optimize the output
e.g.
```{{ states(sensor.solarpower, rounded=True) }} kWh```

### Specific icons per condition

Add an icon per weather condition to the ehmtx component

```yaml
  - id: weather_clear_night
      lameid: 52163
    - id: weather_cloudy
      lameid: 25991
    - id: weather_fog
      lameid: 52167
    ......
```

Sample automation to show the weather with local temperature

```yaml
alias: EHMTX weather
description: weather with icon per condition
trigger:
  - platform: state
    entity_id: weather.metno
action:
  - service: esphome.ulanzi_del_screen
    data:
      icon_name: weather_*
  - service: esphome.ulanzi_screen
    data:
      icon_name: weather_{{ trigger.to_state.state }}
      text: >-
        {{ states("sensor.external_actual_temperature") }}°C
```

or another sample automation for the trashcan type

```yaml
alias: "EHMTX Müllanzeige"
description: Anzeige welche Tonne raus muss. iconnamen gekürzt
trigger:
  - platform: time
    at:
      - "06:30"
      - "08:30"
      - "10:30"
      - "15:00"
      - "17:00"
      - "19:00"
condition:
  - condition: numeric_state
    entity_id: sensor.mulltrigger
    below: "3"
action:
  - service: esphome.ulanzi_del_screen
    data:
      icon_name: trash_*
  - data:
      icon_name: >-
        trash_{{ states("sensor.mulldetails") | replace("Biotonne",   "brow")|
        replace("Papiertonne","blue")| replace("Restmüll",   "grey")|
        replace("gelbe Tonne","yell|") | truncate(4,true,"")  }}     
      text: >-
        {{ states("sensor.mulldetails")|replace(" in","")|replace(" days","
        Tagen") | replace ("0 Tagen","heute") | replace ("1 Tagen","morgen")}}
      duration: 120
    service: esphome.ulanzi_screen
mode: single
```

Prerequisites: This works since 2023.3.1 thanx to @andrew-codechimp fpr the new del_screen

### Integrate in Home Assistant UI

Add entities to Home Assistant UI for interactive control of your display

#### Brightness

```yaml
number:
  - platform: template
    name: "$devicename brightness"
    min_value: 0
    max_value: 255
    step: 1
    lambda: |-
      return id(rgb8x32)->get_brightness();
    set_action:
      lambda: |-
        id(rgb8x32)->set_brightness(x);
```

#### Display switch

```yaml
switch:
  - platform: template
    name: "$devicename Display"
    icon: "mdi:power"
    restore_mode: ALWAYS_ON
    lambda: |-
      return id(rgb8x32)->show_display;
    turn_on_action:
      lambda: |-
        id(rgb8x32)->set_display_on();
    turn_off_action:
      lambda: |-
        id(rgb8x32)->set_display_off();
```

## Buzzer, sound, buttons and automatic brightness

Awtrix and PixelIt have hardcoded functionality. EHMTX is also capable to build something like that by lambdas. But this is all your freedom.

Example: automatic brightness control with an bh1570 sensor

```yaml
sensor:
  - platform: bh1570
    # ...
    on_value:
      then:
         lambda: |-  
            if (x > 200)
            {
               id(rgb8x32)->set_brightness(50);
            } else {
               id(rgb8x32)->set_brightness(250);
            }
```

## Notifier Custom Component
There is an optional [notifier custom component](https://github.com/lubeda/EHMTX_custom_component) you can install with [HACS](https://hacs.xyz/). It is comparable to the **_screen** service but more streamlined.

## Breaking changes

- 2022.6.1 removed image types only `rgb565` is valid!
- 2023.2.0 removed awtrix icon `awtrixid` support
- 2023.3.5 removed automatic scaling of images and animations
- 2023.3.5 added status,display_on,display_off as default service => remove these from your yaml
- 2023.3.5 added indicator_on/off as default services => remove these from your yaml
- 2023.3.5 added *_color as default services => remove these from your yaml
- 2023.3.5 added show_all_icons,gauge_percent/gauge_off as default services => remove these from your yaml

## Usage
The integration works with the Home Assistant api so, after boot of the device, it takes a few seconds until the service calls start working.

## Disclaimer

THE SOFTWARE IS PROVIDED "AS IS", use at your own risk!

## Thanks
- **[blakadder](https://github.com/blakadder)** for his contribution (cleanup README.md,fixed sample)
- **[andrew-codechimp](https://github.com/andrew-codechimp)** for his contribution (display on/off & del_screen "*" & show_clock with 0)
- **[jd1](https://github.com/jd1)** for his contributions
- **[aptonline](https://github.com/aptonline)** for his work on the ulanzi hardware
- **[wsbtak](https://github.com/wsbtak)** for the work on the ulanzi hardware
- **[ofirsnb](https://github.com/ofirsnb)** for his contributions

## Special thanks to all sponsors
