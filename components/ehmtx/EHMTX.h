#ifndef EHMTX_H
#define EHMTX_H
#include "esphome.h"

const uint8_t MAXQUEUE = 24;
const uint8_t MAXICONS = 90;
const uint8_t TEXTSCROLLSTART = 8;
const uint8_t TEXTSTARTOFFSET = (32 - 8);

const uint16_t TICKINTERVAL = 1000; // each 1000ms
static const char *const EHMTX_VERSION = "Version: 2023.4.0";
static const char *const TAG = "EHMTX";

namespace esphome
{
  class EHMTX_screen;
  class EHMTX_store;
  class EHMTX_Icon;
  class EHMTXNextScreenTrigger;
  class EHMTXNextClockTrigger;

  class EHMTX : public PollingComponent, public api::CustomAPIDevice   {
  protected:
    float get_setup_priority() const override { return esphome::setup_priority::AFTER_CONNECTION; }
    uint8_t brightness_;
    bool week_starts_monday;
    bool show_day_of_week;
    std::string time_fmt;
    std::string date_fmt;
    Color indicator_color;
    Color clock_color;
    Color today_color;
    Color weekday_color;
    EHMTX_store *store;
    std::vector<EHMTXNextScreenTrigger *> on_next_screen_triggers_;
    std::vector<EHMTXNextClockTrigger *> on_next_clock_triggers_;
    void internal_add_screen(uint8_t icon, std::string text, uint16_t lifetime,uint16_t show_time, bool alarm);

  public:
    EHMTX();
    Color text_color, alarm_color, gauge_color;
    void dump_config();
    bool show_screen;
    bool show_indicator;
    bool show_gauge;
    bool show_date;
    uint8_t gauge_value;
    bool show_icons;
    void force_screen(std::string name);
    EHMTX_Icon *icons[MAXICONS];
    EHMTX_screen *icon_screen;
    void add_icon(EHMTX_Icon *icon);
    bool show_display;
    bool has_active_screen;
    addressable_light::AddressableLightDisplay *display;
    time::RealTimeClock *clock;
    display::Font *font;
    int8_t yoffset, xoffset;
    uint8_t find_icon(std::string name);
    bool string_has_ending(std::string const &fullString, std::string const &ending);
    bool show_seconds;
    //uint16_t duration;         // in minutes how long is a screen valid
    uint16_t scroll_intervall; // ms to between scrollsteps
    uint16_t frame_intervall;   // ms to next_frame()
    uint16_t clock_time;       // seconds display of screen_time - clock_time = date_time
    uint16_t hold_time;       // seconds display of screen_time to extend 
    uint16_t clock_interval;       // seconds display of screen_time - clock_time = date_time
    uint16_t screen_time;      // seconds display of screen
    uint8_t icon_count;        // max iconnumber -1
    unsigned long last_scroll_time;
    unsigned long last_anim_time;
    time_t last_clock_time = 0;  // starttime clock display
    time_t next_action_time = 0; // when is the next screen change
    void draw_day_of_week();
    void show_all_icons();
    void tick();
    void draw();
    void get_status();
    void skip_screen();
    void hold_screen();
    std::string get_current();
    void set_display(addressable_light::AddressableLightDisplay *disp);
    void set_screen_time(uint16_t t);
    void set_show_clock(uint16_t t);
    void set_hold_time(uint16_t t);
    void set_clock_interval(uint16_t t);
    void set_show_day_of_week(bool b);
    void set_show_seconds(bool b);
    void set_show_date(bool b);
    void set_font_offset(int8_t x, int8_t y);
    void set_week_start(bool b);
    void set_brightness(int b); // int because of register_service!
    uint8_t get_brightness();
    void add_screen(std::string icon, std::string text, int duration, int showt_time, bool alarm);
    void del_screen(std::string iname);
    void set_clock(time::RealTimeClock *clock);
    void set_font(display::Font *font);
    void set_frame_intervall(uint16_t intervall);
    void set_scroll_intervall(uint16_t intervall);
    void set_duration(uint8_t d);
    void set_indicator_off();
    void set_time_format(std::string s);
    void set_date_format(std::string s);
    void set_indicator_on(int r, int g, int b);
    void set_gauge_off();
    void set_gauge_value(int v); // int because of register_service
    void set_gauge_color(int r, int g, int b);
    void set_text_color(int r, int g, int b);
    void set_clock_color(int r, int g, int b);
    void set_today_color(int r, int g, int b);
    void set_weekday_color(int r, int g, int b);
    void set_alarm_color(int r, int g, int b);
    void set_icon_count(uint8_t ic);
    void draw_clock();
    void draw_gauge();
    void add_on_next_screen_trigger(EHMTXNextScreenTrigger *t) { this->on_next_screen_triggers_.push_back(t); }
    void add_on_next_clock_trigger(EHMTXNextClockTrigger *t) { this->on_next_clock_triggers_.push_back(t); }
    void setup();
    void update();
    void set_display_on();
    void set_display_off();
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
    void hold_current(uint _sec);
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
    uint16_t screen_time;
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
    void reset_shiftx();
    bool update_slot(uint8_t _icon);
    void update_screen();
    bool del_slot(uint8_t _icon);
    void hold_slot(uint8_t _sec);
    void set_text(std::string text, uint8_t icon, uint16_t pixel, uint16_t et, uint16_t st);
  };

  class EHMTXNextScreenTrigger : public Trigger<std::string, std::string>
  {
  public:
    explicit EHMTXNextScreenTrigger(EHMTX *parent) { parent->add_on_next_screen_trigger(this); }
    void process(std::string, std::string);
  };

  class EHMTXNextClockTrigger : public Trigger<>
  {
  public:
    explicit EHMTXNextClockTrigger(EHMTX *parent) { parent->add_on_next_clock_trigger(this); }
    void process();
  };

  template <typename... Ts>
  class SetBrightnessAction : public Action<Ts...>
  {
  public:
    SetBrightnessAction(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, brightness)

    void play(Ts... x) override
    {
      auto brightness = this->brightness_.value(x...);

      this->parent_->set_brightness(brightness);
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class AddScreenAction : public Action<Ts...>
  {
  public:
    AddScreenAction(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(std::string, icon)
    TEMPLATABLE_VALUE(std::string, text)
    TEMPLATABLE_VALUE(uint8_t, lifetime)
    TEMPLATABLE_VALUE(uint16_t, screen_time)
    TEMPLATABLE_VALUE(bool, alarm)

    void play(Ts... x) override
    {
      auto icon = this->icon_.value(x...);
      auto text = this->text_.value(x...);
      auto lifetime = this->lifetime_.value(x...);
      auto screen_time = this->screen_time_.value(x...);
      auto alarm = this->alarm_.value(x...);

      this->parent_->add_screen(icon, text, lifetime, screen_time, alarm);
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
      this->parent_->set_indicator_on(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
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
  class SetAlarmColor : public Action<Ts...>
  {
  public:
    SetAlarmColor(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, red)
    TEMPLATABLE_VALUE(uint8_t, green)
    TEMPLATABLE_VALUE(uint8_t, blue)

    void play(Ts... x) override
    {
      this->parent_->set_alarm_color(this->red_.value(x...), this->green_.value(x...), this->blue_.value(x...));
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
  class SetShowDate : public Action<Ts...>
  {
  public:
    SetShowDate(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, flag)

    void play(Ts... x) override
    {
      this->parent_->set_show_date(this->flag_.value(x...));
    }

  protected:
    EHMTX *parent_;
  };

  template <typename... Ts>
  class SetShowDayOfWeek : public Action<Ts...>
  {
  public:
    SetShowDayOfWeek(EHMTX *parent) : parent_(parent) {}
    TEMPLATABLE_VALUE(uint8_t, flag)

    void play(Ts... x) override
    {
      this->parent_->set_show_day_of_week(this->flag_.value(x...));
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
  class SetDisplayOn : public Action<Ts...>
  {
  public:
    SetDisplayOn(EHMTX *parent) : parent_(parent) {}

    void play(Ts... x) override
    {
      this->parent_->set_display_on();
    }

  protected:
    EHMTX *parent_;
  };

   template <typename... Ts>
  class SetDisplayOff : public Action<Ts...>
  {
  public:
    SetDisplayOff(EHMTX *parent) : parent_(parent) {}

    void play(Ts... x) override
    {
      this->parent_->set_display_off();
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
    bool fullscreen;
    void next_frame();
    bool reverse;
  };
}

#endif
