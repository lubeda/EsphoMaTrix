#ifndef EHMTX_H

#define EHMTX_H
#include "esphome.h"

#define MAXQUEUE 16
#define MAXICONS 48
#define TEXTSCROLLSTART 9
#define TICKINTERVAL 1000 // each 100ms
#define VERSION "Version: 3.2.2022"

namespace esphome
{

  const Color EHMTX_Cwarn = Color(214, 214, 12);
  const Color EHMTX_Calarm = Color(204, 51, 63);
  const Color EHMTX_cday = Color(60, 60, 60);

  class EHMTX_screen;

  class EHMTX : public PollingComponent 
  {
  private:
    uint8_t findfreeslot(uint8_t icon);
    float get_setup_priority() const override { return esphome::setup_priority::LATE; }
    Color indicatorColor;

  public:
    EHMTX();
    Color textColor, alarmColor;
    bool showscreen;
    bool showindicator;
    EHMTX_screen *slots[MAXQUEUE];
    display::Animation *icons[MAXICONS];
    const char *iconnames[MAXICONS];
    void add_icon(display::Animation *icon,const char *name);
    display::DisplayBuffer *display;
    time::RealTimeClock *clock;
    display::Font *font;
    int8_t yoffset,xoffset;
    uint8_t find_icon(std::string name);
    int8_t duration;          // in minutes how long is a screen valid
    uint16_t scrollintervall; // ms to between scrollsteps
    uint16_t animintervall;   // ms to next_frame()
    uint16_t clocktime;       // ms display of clock/date 0.5 clock then 0.5 date
    uint16_t screentime;      // ms display of screen
    uint8_t iconcount;        // max iconnumber -1
    uint8_t activeslot;       // slot to display
    unsigned long lastscrolltime;
    unsigned long lastanimtime;
    time_t lastclocktime = 0;  // starttime clock display
    time_t nextactiontime = 0; // when is the nextscreenchange
    uint8_t findnextscreen();
    uint8_t countscreens();
    void drawdayofweek();
    void tick();
    void draw();
    void get_status();
    void set_display(display::DisplayBuffer *disp);
    void set_screentime(uint16_t t);
    void set_fontoffset(int8_t x,int8_t y );
    void set_clocktime(uint16_t t);
    void add_alarm(uint8_t icon, std::string text);
    void add_screen(uint8_t icon, std::string text);
    void add_screen_n(std::string icon, std::string text);
    void add_screen_u(std::string icon, std::string text,uint16_t duration, bool alarm);
    void add_screen_t(uint8_t icon, std::string text,uint16_t t);
    void del_screen(uint8_t icon);
    void del_screen_n(std::string iname);
    void set_clock(time::RealTimeClock *clock);
    void set_font(display::Font *font);
    void set_animintervall(uint16_t intervall);
    void set_scrollintervall(uint16_t intervall);
    void set_duration(uint8_t d);
    void set_indicatoroff();
    void set_indicatoron();
    void set_indicatorcolor(int r, int g, int b);
    void set_textcolor(int r, int g, int b);
    void set_alarmcolor(int r, int g, int b);
    void set_iconcount(uint8_t ic);
    void drawclock();
    void setup();
    void update();
  };

  class EHMTX_screen
  {

  private:
    uint8_t shiftx;
    uint8_t pixels;
    EHMTX *config;

  public:
    bool alarm;
    time_t endtime;
    uint8_t icon;
    std::string text;

    EHMTX_screen(EHMTX *config);

    bool active();
    bool isalarm();
    void draw();
    void _draw();
    bool isfree();
    bool updateslot(uint8_t _icon);
    void update_screen();
    bool delslot(uint8_t _icon);
    //void setText(std::string text, uint8_t icon, uint8_t pixel);
    void setText(std::string text, uint8_t icon, uint8_t pixel,uint16_t et);
  };

}

#endif