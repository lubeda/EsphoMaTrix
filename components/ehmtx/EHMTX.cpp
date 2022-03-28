#include "esphome.h"

namespace esphome
{
  EHMTX::EHMTX() : PollingComponent(TICKINTERVAL)
  {
    this->store = new EHMTX_store(this);
    this->show_screen = false;
    this->show_gauge = false;
    this->gauge_value = 0;
    this->icon_count = 0;
    this->text_color = Color(240, 240, 240);
    this->today_color = Color(240, 240, 240);
    this->weekday_color = Color(100, 100, 100);
    this->clock_color = Color(240, 240, 240);
    this->alarm_color = Color(200, 50, 50);
    this->gauge_color = Color(100, 100, 200);
    this->last_clock_time = 0;
    this->icon_screen = new EHMTX_screen(this);
#ifdef USE_EHMTX_SELECT
    this->select = NULL;
#endif
  }

  void EHMTX::force_screen(std::string name)
  {
    uint8_t icon_id = this->find_icon(name);
    if (icon_id < MAXICONS)
    {
      this->store->force_next_screen(icon_id);
      ESP_LOGD(TAG, "force next screen: %s", name.c_str());
    }
  }

  void EHMTX::set_indicator_color(int r, int g, int b)
  {
    this->indicator_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGD(TAG, "indicator r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_today_color(int r, int g, int b)
  {
    this->today_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGD("EHMTX", "Today r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_weekday_color(int r, int g, int b)
  {
    this->weekday_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGD("EHMTX", "Weekday r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_clock_color(int r, int g, int b)
  {
    this->clock_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGD("EHMTX", "clock r: %d g: %d b: %d", r, g, b);
  }


  void EHMTX::set_gauge_color(int r, int g, int b)
  {
    this->gauge_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGD(TAG, "gauge r: %d g: %d b: %d", r, g, b);
  }

  uint8_t EHMTX::find_icon(std::string name)
  {
    for (uint8_t i = 0; i < this->icon_count; i++)
    {
      if (strcmp(this->icons[i]->name.c_str(), name.c_str()) == 0)
      {
        ESP_LOGD(TAG, "icon: %s found id: %d", name.c_str(), i);
        return i;
      }
    }
    ESP_LOGD(TAG, "icon: %s not found", name.c_str());
    return MAXICONS;
  }

  void EHMTX::set_alarm_color(int r, int g, int b)
  {
    this->alarm_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  void EHMTX::set_text_color(int r, int g, int b)
  {
    this->text_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  void EHMTX::set_indicator_off()
  {
    this->show_indicator = false;
    ESP_LOGD(TAG, "indicator off");
  }
  void EHMTX::set_indicator_on()
  {
    this->show_indicator = true;
    ESP_LOGD(TAG, "indicator on");
  }

  void EHMTX::set_gauge_off()
  {
    this->show_gauge = false;
    ESP_LOGD(TAG, "gauge off");
  }
  void EHMTX::set_gauge_value(uint8_t val)
  {
    this->show_gauge = false;
    if (val > 0)
    {
      this->show_gauge = true;
      this->gauge_value = (uint8_t)(100-val) * 7 / 100;
      ESP_LOGD(TAG, "gauge value: %d", this->gauge_value);
    }
  }

  void EHMTX::draw_clock()
  {
    if ((this->clock->now().timestamp - this->next_action_time) < this->clock_time)
    {
      this->display->strftime(this->xoffset + 15, this->yoffset, this->font, this->clock_color, display::TextAlign::BASELINE_CENTER, "%H:%M",
                              this->clock->now());
    }
    else
    {
      this->display->strftime(this->xoffset + 15, this->yoffset, this->font, this->clock_color, display::TextAlign::BASELINE_CENTER, "%d.%m.",
                              this->clock->now());
    }
    this->draw_day_of_week();
  }

  void EHMTX::setup()
  {
#ifdef USE_EHMTX_SELECT
    if (this->select != NULL)
    {
      ESP_LOGD(TAG, "use select_component");
      this->select->traits.set_options(this->select_options);
      this->select->parent = this;
    }
#endif
  }

  void EHMTX::update()
  {
    time_t ts = this->clock->now().timestamp;
    if ((this->next_action_time + 15) < ts)
    {
      this->next_action_time = ts + 3;
      this->last_clock_time = ts;
    }
  }
  void EHMTX::skip_screen()
  {
    this->next_action_time = 1;
  }

  void EHMTX::tick()
  {

    time_t ts = this->clock->now().timestamp;
    if ((ts - this->next_action_time) > this->screen_time)
    {
      this->next_action_time = ts + this->screen_time;
      if (this->show_icons)
      {
        uint8_t i = this->icon_screen->icon;
        ++i;
        if (i < this->icon_count)
        {
          int x, y, w, h;
          this->display->get_text_bounds(0, 0, this->icons[i]->name.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
          this->icon_screen->set_text(this->icons[i]->name, i, w, 1);
          ESP_LOGD(TAG, "show all icons icon: %d name: %s", i, this->icons[i]->name.c_str());
        }
        else
        {
          this->show_icons = false;
          ESP_LOGD(TAG, "show all icons done");
        }
      }
      else
      {
        this->show_screen = false;

        if (!(ts - this->last_clock_time > 60))
        {
          bool has_next_screen = this->store->move_next();
          if (has_next_screen)
          {
            this->show_screen = true;
          }
        }
        if (this->show_screen == false)
        {
          this->last_clock_time = this->clock->now().timestamp;
          this->next_action_time = ts + this->screen_time;
        }
        else
        {
          this->next_action_time = ts + (int)this->store->current()->display_duration;
          for (auto *t : on_next_screen_triggers_)
          {
            t->process(this->icons[this->store->current()->icon]->name, this->store->current()->text);
          }
        }
      }
    }
  }

  void EHMTX::set_screen_time(uint16_t t)
  {
    this->screen_time = t;
  }

  void EHMTX::set_duration(uint8_t t)
  {
    this->duration = t;
  }

  void EHMTX::get_status()
  {
    time_t ts = this->clock->now().timestamp;
    ESP_LOGI(TAG, "status time: %d.%d.%d %02d:%02d", this->clock->now().day_of_month,
             this->clock->now().month, this->clock->now().year,
             this->clock->now().hour, this->clock->now().minute);
    ESP_LOGI(TAG, "status brightness: %d (0..255)", this->brightness_);
    ESP_LOGI(TAG, "status default duration: %d", this->duration);
    ESP_LOGI(TAG, "status text_color: RGB(%d,%d,%d)", this->text_color.r, this->text_color.g, this->text_color.b);
    ESP_LOGI(TAG, "status alarm_color: RGB(%d,%d,%d)", this->alarm_color.r, this->alarm_color.g, this->alarm_color.b);
    if (this->show_indicator)
    {
      ESP_LOGI(TAG, "status indicator on");
    }
    else
    {
      ESP_LOGI(TAG, "status indicator off");
    }

    this->store->log_status();

    for (uint8_t i = 0; i < this->icon_count; i++)
    {
      ESP_LOGI(TAG, "status icon: %d name: %s", i, this->icons[i]->name.c_str());
    }
#ifdef USE_EHMTX_SELECT
    ESP_LOGI(TAG, "select enabled");
#endif
  }

  void EHMTX::set_font(display::Font *font)
  {
    this->font = font;
  }

  void EHMTX::set_anim_intervall(uint16_t ai)
  {
    this->anim_intervall = ai;
  }

  void EHMTX::set_scroll_intervall(uint16_t si)
  {
    this->scroll_intervall = si;
  }

  void EHMTX::del_screen(std::string iname)
  {
    uint8_t icon = this->find_icon(iname.c_str());
    this->store->delete_screen(icon);
  }

  void EHMTX::add_screen(std::string iconname, std::string text, uint16_t duration, bool alarm)
  {
    uint8_t icon = this->find_icon(iconname.c_str());
    this->internal_add_screen(icon, text, duration, alarm);
    ESP_LOGD(TAG, "add_screen icon: %d iconname: %s text: %s duration: %d alarm: %d", icon, iconname.c_str(), text.c_str(), duration, alarm);
  }

  void EHMTX::internal_add_screen(uint8_t icon, std::string text, uint16_t duration, bool alarm = false)
  {
    if (icon >= this->icon_count)
    {
      ESP_LOGD(TAG, "icon %d not found => default: 0", icon);
      icon = 0;
    }
    EHMTX_screen *screen = this->store->find_free_screen(icon);

    int x, y, w, h;
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    screen->alarm = alarm;
    screen->set_text(text, icon, w, duration);
  }

  void EHMTX::set_default_brightness(uint8_t b)
  {
    this->brightness_ = b;
  }

  void EHMTX::set_week_start(bool b)
  {
    this->week_starts_monday = b;
    if (b){
      ESP_LOGI(TAG, "weekstart: monday");
    } else {
      ESP_LOGI(TAG, "weekstart: sunday");
    } 
  }

  void EHMTX::set_brightness(uint8_t b)
  {
    this->brightness_ = b;
    float br = (float)b / (float)255;
    ESP_LOGI(TAG, "set_brightness %d => %.2f %%", b, 100 * br);
    this->display->get_light()->set_correction(br, br, br, br);
  }

  uint8_t EHMTX::get_brightness()
  {
    return this->brightness_;
  }

  std::string EHMTX::get_current()
  {
    return this->icons[this->store->current()->icon]->name;
  }

  void EHMTX::set_clock_time(uint16_t t)
  {
    this->clock_time = t;
  }

  void EHMTX::set_display(addressable_light::AddressableLightDisplay *disp)
  {
    this->display = disp;
  }

  void EHMTX::set_clock(time::RealTimeClock *clock)
  {
    this->clock = clock;
    this->store->clock = clock;
  }

  void EHMTX::draw_day_of_week()
  {
    auto dow = this->clock->now().day_of_week - 1; // SUN = 0
      for (uint8_t i = 0; i <= 6; i++)
      {
        if ( ((!this->week_starts_monday) && (dow == i)) || 
             ((this->week_starts_monday) && ((dow == (i+1)) || ((dow==0 && i == 6)) )))
        {
          this->display->line(2 + i * 4, 7, i * 4 + 4, 7, this->today_color);
        }
        else
        {
          this->display->line(2 + i * 4, 7, i * 4 + 4, 7, this->weekday_color);
        }
      }
    
  };

  void EHMTX::set_font_offset(int8_t x, int8_t y)
  {
    this->xoffset = x;
    this->yoffset = y;
  }

  void EHMTX::dump_config()
  {
    ESP_LOGCONFIG(TAG, "EspHoMatriX %s", EHMTX_VERSION);
    ESP_LOGCONFIG(TAG, "Icons: %d of %d", this->icon_count, MAXICONS);
    ESP_LOGCONFIG(TAG, "Font offset: x=%d y=%d", this->xoffset, this->yoffset);
    ESP_LOGCONFIG(TAG, "Max screens: %d", MAXQUEUE);
    ESP_LOGCONFIG(TAG, "Intervall (ms) scroll: %d anim: %d", this->scroll_intervall, this->anim_intervall);
    ESP_LOGCONFIG(TAG, "Displaytime (s) clock: %d screen: %d", this->clock_time, this->screen_time);
    if (this->week_starts_monday){
      ESP_LOGCONFIG(TAG, "weekstart: monday");
    } else {
      ESP_LOGCONFIG(TAG, "weekstart: sunday");
    } 
  }

#ifdef USE_EHMTX_SELECT
  void EHMTX::set_select(esphome::EhmtxSelect *es)
  {
    this->select = es;
  }
#endif

  void EHMTX::add_icon(EHMTX_Icon *icon)
  {
    this->icons[this->icon_count] = icon;
    ESP_LOGD(TAG, "add_icon no.: %d name: %s duration: %d ", this->icon_count, icon->name.c_str(), icon->frame_duration);
    this->icon_count++;

#ifdef USE_EHMTX_SELECT
    this->select_options.push_back(icon->name);
#endif
  }

  void EHMTX::show_all_icons()
  {
    int x, y, w, h;
    ESP_LOGD(TAG, "show all icons icon: %s", this->icons[0]->name.c_str());
    this->display->get_text_bounds(0, 0, this->icons[0]->name.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    this->icon_screen->set_text(this->icons[0]->name, 0, w, 1);
    this->show_icons = true;
  }

  void EHMTX::draw()
  {
    if (this->show_icons)
    {
      this->icon_screen->draw();
    }
    else
    {

      if (this->show_screen)
      {
        this->store->current()->draw();
      }
      else
      {
        this->draw_clock();
      }
    }
    
    if (this->show_indicator)
    {
      this->display->line(31, 5, 29, 7, this->indicator_color);
      this->display->draw_pixel_at(30, 7, this->indicator_color);
      this->display->draw_pixel_at(31, 6, this->indicator_color);
      this->display->draw_pixel_at(31, 7, this->indicator_color);
    }
  }

  /* Trigger */

  void EHMTXNextScreenTrigger::process(std::string iconname, std::string text)
  {
    this->trigger(iconname, text);
  }

}
