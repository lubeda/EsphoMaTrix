#ifndef EHMTX_H

#define EHMTX_H
#include "esphome.h"

const uint8_t MAXQUEUE = 24;
const uint8_t MAXICONS = 64;
const uint8_t TEXTSCROLLSTART = 8;
const uint8_t TEXTSTARTOFFSET = (32-8);

const uint16_t TICKINTERVAL = 1000; // each 1000ms
static const char *const EHMTX_VERSION = "Version: 2022.3.11 (RGB)";
static const char *const TAG = "EHMTX";

namespace esphome
{
  class EHMTX_screen;
  class EHMTX_store;
  class EhmtxSelect;
  class EHMTX_Icon;
  class EHMTXNextScreenTrigger;

  class EHMTX : public PollingComponent
  {
  protected:
    float get_setup_priority() const override { return esphome::setup_priority::AFTER_CONNECTION; }
    uint8_t brightness_;
    Color indicator_color;
    Color clock_color;
    Color today_color;
    Color weekday_color;
    EHMTX_store *store;
    std::vector<EHMTXNextScreenTrigger *> on_next_screen_triggers_;
    void internal_add_screen(uint8_t icon, std::string text, uint16_t duration, bool alarm);

  public:
    EHMTX();
    Color text_color, alarm_color,gauge_color;
    void dump_config();
    bool show_screen;
    bool show_indicator;
    bool show_gauge;
    bool week_starts_monday;
    uint8_t gauge_value;
    bool show_icons;
    void force_screen(std::string name);
    EHMTX_Icon *icons[MAXICONS];
    EHMTX_screen *icon_screen;
    void add_icon(EHMTX_Icon *icon);
#ifdef USE_EHMTX_SELECT
    std::vector<std::string> select_options;
    esphome::EhmtxSelect *select;
    void set_select(esphome::EhmtxSelect *es);
#endif
    addressable_light::AddressableLightDisplay *display;
    time::RealTimeClock *clock;
    display::Font *font;
    int8_t yoffset, xoffset;
    uint8_t find_icon(std::string name);
    uint16_t duration;         // in minutes how long is a screen valid
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
    void show_all_icons();
    void tick();
    void draw();
    void get_status();
    void skip_screen();
    std::string get_current();
    void set_display(addressable_light::AddressableLightDisplay *disp);
    void set_screen_time(uint16_t t);
    void set_font_offset(int8_t x, int8_t y);
    void set_clock_time(uint16_t t);
    void set_week_start(bool b);
    void set_default_brightness(uint8_t b);
    void set_brightness(uint8_t b);
    uint8_t get_brightness();
    void add_screen(std::string icon, std::string text, uint16_t duration, bool alarm);
    void del_screen(std::string iname);
    void set_clock(time::RealTimeClock *clock);
    void set_font(display::Font *font);
    void set_anim_intervall(uint16_t intervall);
    void set_scroll_intervall(uint16_t intervall);
    void set_duration(uint8_t d);
    void set_indicator_off();
    void set_indicator_on();
    void set_indicator_color(int r, int g, int b);
    void set_gauge_off();
    void set_gauge_value(uint8_t v);
    void set_gauge_color(int r, int g, int b);
    void set_text_color(int r, int g, int b);
    void set_clock_color(int r, int g, int b);
    void set_today_color(int r, int g, int b);
    void set_weekday_color(int r, int g, int b); 
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
    uint8_t force_screen;
    uint8_t count_active_screens();

  public:
    EHMTX_store(EHMTX *config);
    void force_next_screen(uint8_t icon_id);
    time::RealTimeClock *clock;
    EHMTX_screen *find_free_screen(uint8_t icon);
    void delete_screen(uint8_t icon);
    bool move_next();
    EHMTX_screen *current();
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
    void set_text(std::string text, uint8_t icon, uint8_t pixel, uint16_t et);
  };

  class EHMTXNextScreenTrigger : public Trigger<std::string, std::string>
  {
  public:
    explicit EHMTXNextScreenTrigger(EHMTX *parent) { parent->add_on_next_screen_trigger(this); }
    void process(std::string, std::string);
  };

  template <typename... Ts>
  class AddScreenAction : public Action<Ts...>
  {
  public:
    AddScreenAction(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(std::string, icon)
    TEMPLATABLE_VALUE(std::string, text)
    TEMPLATABLE_VALUE(uint8_t, duration)
    TEMPLATABLE_VALUE(bool, alarm)

    void play(Ts... x) override
    {
      auto icon = this->icon_.value(x...);
      auto text = this->text_.value(x...);
      auto duration = this->duration_.value(x...);
      auto alarm = this->alarm_.value(x...);

      if(duration) {
        this->parent_->add_screen(icon, text, duration, alarm);
      } else {
        this->parent_->add_screen(icon, text, this->parent_->duration, alarm);
      }
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class SetIndicatorOn : public Action<Ts...>
  {
  public:
    SetIndicatorOn(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_indicator_on();
      this->parent_->set_indicator_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class SetClockColor : public Action<Ts...>
  {
  public:
    SetClockColor(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_clock_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

template <typename... Ts>
  class SetTodayColor : public Action<Ts...>
  {
  public:
    SetTodayColor(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_today_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

template <typename... Ts>
  class SetTextColor : public Action<Ts...>
  {
  public:
    SetTextColor(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_text_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };


template <typename... Ts>
  class SetWeekdayColor : public Action<Ts...>
  {
  public:
    SetWeekdayColor(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_weekday_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };


  template <typename... Ts>
  class SetIndicatorOff : public Action<Ts...>
  {
  public:
    SetIndicatorOff(EHMTX *parent) : parent_(parent) {}

    void play(Ts... x) override
    {
      this->parent_->set_indicator_off();
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class DeleteScreen : public Action<Ts...>
  {
  public:
    DeleteScreen(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(std::string, icon)

    void play(Ts... x) override
    {
        this->parent_->del_screen(this->icon_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class ForceScreen : public Action<Ts...>
  {
  public:
    ForceScreen(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(std::string, icon)

    void play(Ts... x) override
    {
      this->parent_->force_screen(this->icon_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

  class EHMTX_Icon : public display::Animation
  {
  protected:
    bool counting_up;

  public:
    EHMTX_Icon(const uint8_t *data_start, int width, int height, uint32_t animation_frame_count, display::ImageType type, std::string icon_name, bool revers, uint16_t frame_duration);
    std::string name;
    uint16_t frame_duration;
    void next_frame();
    bool reverse;
  };

}

#endif