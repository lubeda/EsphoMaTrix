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
  class EHMTX_store;
  class EHMTXNextScreenTrigger;

  class EHMTX : public PollingComponent 
  {
  protected:
    float get_setup_priority() const override { return esphome::setup_priority::AFTER_CONNECTION; }
    uint8_t brightness_;
    Color indicator_color;
    EHMTX_store* store;
    std::vector<EHMTXNextScreenTrigger *> on_next_screen_triggers_;
    void internal_add_screen(uint8_t icon, std::string text, uint16_t duration, bool alarm);

  public:
    EHMTX();
    Color text_color, alarm_color;
    void dump_config();
    bool show_screen;
    bool show_indicator;
    display::Animation *icons[MAXICONS];
    const char *iconnames[MAXICONS];
    void add_icon(display::Animation *icon,const char *name);
    addressable_light::AddressableLightDisplay *display;
    time::RealTimeClock *clock;
    display::Font *font;
    int8_t yoffset,xoffset;
    uint8_t find_icon(std::string name);
    uint8_t duration;          // in minutes how long is a screen valid
    uint16_t scroll_intervall; // ms to between scrollsteps
    uint16_t anim_intervall;   // ms to next_frame()
    uint16_t clock_time;       // ms display of clock/date 0.5 clock then 0.5 date
    uint16_t screen_time;      // ms display of screen
    uint8_t icon_count;        // max iconnumber -1
    unsigned long last_scroll_time;
    unsigned long last_anim_time;
    time_t last_clock_time = 0;  // starttime clock display
    time_t next_action_time = 0; // when is the nextscreenchange
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
    void add_on_next_screen_trigger(EHMTXNextScreenTrigger *t) { this->on_next_screen_triggers_.push_back(t); }
    void setup();
    void update();
  };

  class EHMTX_store
  {
    protected:
      EHMTX_screen *slots[MAXQUEUE];
      uint8_t active_slot;
      uint8_t count_active_screens();
    public:
      EHMTX_store(EHMTX *config);
      time::RealTimeClock *clock;
      EHMTX_screen* find_free_screen(uint8_t icon);
      void delete_screen(uint8_t icon);
      bool move_next();
      EHMTX_screen* current();
      void log_status();
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


  
class EHMTXNextScreenTrigger : public Trigger<std::string,std::string> {
 public:
  explicit EHMTXNextScreenTrigger(EHMTX *parent) { parent->add_on_next_screen_trigger(this); }
  void process(std::string,std::string);
};


template<typename... Ts> class AddScreenAction : public Action<Ts...> {
 public:
  AddScreenAction(EHMTX *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, icon)
  TEMPLATABLE_VALUE(std::string, text)
  TEMPLATABLE_VALUE(uint8_t, duration)
  TEMPLATABLE_VALUE(bool, alarm)

  void play(Ts... x) override {
    this->parent_->add_screen_u(this->icon_.value(x...), this->text_.value(x...), this->duration_.value(x...),
                           this->alarm_.value(x...));
  }

 protected:
  EHMTX *parent_;
};


template<typename... Ts> class SetIndicatorOn : public Action<Ts...> {
 public:
  SetIndicatorOn(EHMTX *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(uint8_t, red)
  TEMPLATABLE_VALUE(uint8_t, green)
  TEMPLATABLE_VALUE(uint8_t, blue)

  void play(Ts... x) override {
    this->parent_->set_indicator_on();
    this->parent_->set_indicator_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
  }

 protected:
  EHMTX *parent_;
};

template<typename... Ts> class SetIndicatorOff : public Action<Ts...> {
 public:
  SetIndicatorOff(EHMTX *parent) : parent_(parent) {}
  
  void play(Ts... x) override {
    this->parent_->set_indicator_off();
  }

 protected:
  EHMTX *parent_;
};


template<typename... Ts> class DeleteScreen : public Action<Ts...> {
 public:
  DeleteScreen(EHMTX *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, icon)
  
  void play(Ts... x) override {
    this->parent_->del_screen(this->parent_->find_icon(this->icon_.value(x...)));
  }

 protected:
  EHMTX *parent_;
};


}

#endif