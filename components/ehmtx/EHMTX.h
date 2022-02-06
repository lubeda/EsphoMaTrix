#ifndef EHMTX_H

#define EHMTX_H
#include "esphome.h"

const uint8_t MAXQUEUE=16;
const uint8_t MAXICONS=48;
const uint8_t TEXTSCROLLSTART=9;
const uint16_t TICKINTERVAL=1000; // each 1000ms
static const char *const EHMTX_VERSION="Version: 2022.2.7";
static const char *const TAG="EHMTX";

namespace esphome
{
  const Color EHMTX_cday = Color(90, 90, 90);

  class EHMTX_screen;

  class EHMTX : public PollingComponent 
  {
  protected:
    EHMTX_screen* find_free_slot(uint8_t icon);
    float get_setup_priority() const override { return esphome::setup_priority::AFTER_CONNECTION; }
    uint8_t brightness_;
    Color indicator_color;
  public:
    EHMTX();
    Color text_color, alarm_color;
    void dump_config();
    bool show_screen;
    bool showindicator;
    EHMTX_screen *slots[MAXQUEUE];
    display::Animation *icons[MAXICONS];
    const char *iconnames[MAXICONS];
    void add_icon(display::Animation *icon,const char *name);
    addressable_light::AddressableLightDisplay *display;
    time::RealTimeClock *clock;
    display::Font *font;
    int8_t yoffset,xoffset;
    uint8_t find_icon(std::string name);
    int8_t duration;          // in minutes how long is a screen valid
    uint16_t scroll_intervall; // ms to between scrollsteps
    uint16_t anim_intervall;   // ms to next_frame()
    uint16_t clock_time;       // ms display of clock/date 0.5 clock then 0.5 date
    uint16_t screen_time;      // ms display of screen
    uint8_t icon_count;        // max iconnumber -1
    uint8_t active_slot;       // slot to display
    unsigned long last_scroll_time;
    unsigned long last_anim_time;
    time_t last_clock_time = 0;  // starttime clock display
    time_t next_action_time = 0; // when is the nextscreenchange
    uint8_t find_next_screen();
    uint8_t count_screens();
    void draw_day_of_week();
    void tick();
    void draw();
    void get_status();
    void set_display(addressable_light::AddressableLightDisplay *disp); 
    void set_screen_time(uint16_t t);
    void set_font_offset(int8_t x,int8_t y );
    void set_clock_time(uint16_t t);
    void set_default_brightness(uint8_t b);
    void set_brightness(uint8_t b);
    void add_alarm(uint8_t icon, std::string text);
    void add_screen(uint8_t icon, std::string text);
    void add_screen_n(std::string icon, std::string text);
    void add_screen_u(std::string icon, std::string text,uint16_t duration, bool alarm);
    void add_screen_t(uint8_t icon, std::string text,uint16_t t);
    void del_screen(uint8_t icon);
    void del_screen_n(std::string iname);
    void set_clock(time::RealTimeClock *clock);
    void set_font(display::Font *font);
    void set_anim_intervall(uint16_t intervall);
    void set_scroll_intervall(uint16_t intervall);
    void set_duration(uint8_t d);
    void set_indicator_off();
    void set_indicator_on();
    void set_indicator_color(int r, int g, int b);
    void set_text_color(int r, int g, int b);
    void set_alarm_color(int r, int g, int b);
    void set_icon_count(uint8_t ic);
    void draw_clock();
    void setup();
    void update();
  };

  class EHMTX_screen
  {

  protected:
    uint8_t shiftx_;
    uint8_t pixels_;
    EHMTX *config_;

  public:
    float display_duration;
    bool alarm;
    time_t endtime;
    uint8_t icon;
    std::string text;

    EHMTX_screen(EHMTX *config);

    bool active();
    bool is_alarm();
    void draw();
    void draw_();
    bool isfree();
    bool update_slot(uint8_t _icon);
    void update_screen();
    bool del_slot(uint8_t _icon);
    void set_text(std::string text, uint8_t icon, uint8_t pixel,uint16_t et);
  };

}

#endif