#include "esphome.h"

namespace esphome
{

  EHMTX::EHMTX() : PollingComponent(TICKINTERVAL)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      this->slots[i] = new EHMTX_screen(this);
    }
    this->show_screen = false;
    this->active_slot = 0;
    this->icon_count = 0;
    this->text_color = Color(240, 240, 240);
    this->alarm_color = Color(200, 50, 50);
    this->last_clock_time = 0;
  }

  void EHMTX::set_indicator_color(int r, int g, int b)
  {
    this->indicator_color = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  uint8_t EHMTX::find_icon(std::string name)
  {
    for (uint8_t i = 0; i < this->icon_count; i++)
    { 
      if (strcmp (this->iconnames[i],name.c_str()) == 0){
        ESP_LOGD(TAG,"icon: %s found id: %d",name.c_str(),i);
        return i;
      }
    }
    ESP_LOGD(TAG,"icon: %s not found",name.c_str());
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
    this->showindicator = false;
  }
  void EHMTX::set_indicator_on()
  {
    this->showindicator = true;
  }

  void EHMTX::draw_clock()
  {
    if ((this->clock->now().timestamp - this->next_action_time) < this->clocktime) 
    {
      this->display->strftime(6+this->xoffset, this->yoffset, this->font, this->text_color, "%H:%M",
                              this->clock->now());
    }
    else
    {
      this->display->strftime(5+this->xoffset, this->yoffset, this->font, this->text_color, "%d.%m.",
                              this->clock->now());
    }
    this->draw_day_of_week();
  }

  void EHMTX::setup()
  {
  }

  uint8_t EHMTX::count_screens()
  {
    uint8_t count = 0;
    for (uint8_t screen = 0; screen < MAXQUEUE; screen++)
    {
      if (this->slots[screen]->active())
      {
        count++;
      }
    }
    return count;
  }

  uint8_t EHMTX::find_next_screen()
  {
    uint8_t screen = MAXQUEUE;

    if (this->count_screens()== 1){
      for (screen = 0; screen < MAXQUEUE; screen++)
      {
        if (this->slots[screen]->active())
        {
          return screen;
        }
      }  
    }

    for (screen = (this->active_slot + 1); screen < MAXQUEUE; screen++)
    {
      if (this->slots[screen]->active())
      {
        return screen;
      }
    }
    for (screen = 0; screen < this->active_slot; screen++)
    {
      if (this->slots[screen]->active())
      {
        return screen;
      }
    }
    return MAXQUEUE;
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
  
  void EHMTX::tick()
  {
    
    time_t ts = this->clock->now().timestamp;
    
    if ((ts - this->next_action_time) > this->screentime) 
    {
      
      this->next_action_time = ts + this->screentime;

      this->show_screen = false;

      if (!(ts - this->last_clock_time > 60))
      {
        if (this->count_screens() > 0)
        {
          
          uint8_t fns = this->find_next_screen();
          if (fns < MAXQUEUE)
          {
            this->active_slot = fns;
            this->show_screen = true;
          }
        }
      }
      if (this->show_screen == false)
      {
        this->last_clock_time = this->clock->now().timestamp;
      }
    }
  }

  void EHMTX::set_screen_time(uint16_t t)
  {
    this->screentime = t;
  }

  void EHMTX::set_duration(uint8_t t)
  {
    this->duration = t;
  }

  void EHMTX::get_status()
  {
    time_t ts = this->clock->now().timestamp;
    ESP_LOGI(TAG, "status active slot: %d", this->active_slot);
    ESP_LOGI(TAG, "status brightness: %2.1f", this->brightness_);
    ESP_LOGI(TAG, "status screen count: %d", this->count_screens());
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->active())
      {
        int td = this->slots[i]->endtime - ts;
        ESP_LOGI(TAG, "status slot: %d icon: %d iconname: %s  text: %s end: %d sec", i, this->slots[i]->icon,this->iconnames[this->slots[i]->icon], this->slots[i]->text.c_str(), td);
      }
    }
    for (uint8_t i = 0; i < this->icon_count; i++)
    {
      ESP_LOGI(TAG, "status icon: %d name: %s", i, this->iconnames[i]);
    }
  }

  uint8_t EHMTX::find_free_slot(uint8_t icon)
  {
    ESP_LOGD(TAG,"find_free_slot for icon: %d",icon);
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->icon == icon)
      {
        return i;
      }
    }
    time_t ts = this->clock->now().timestamp;
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->endtime <= ts)
      {
        return i;
      }
    }
    return 0;
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

  void EHMTX::del_screen_n(std::string iname)
  {
    uint8_t icon = this->find_icon(iname.c_str());
    this->del_screen(icon);
  }

  void EHMTX::del_screen(uint8_t icon)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      this->slots[i]->del_slot(icon);
    }
  }

  void EHMTX::add_alarm(uint8_t icon, std::string text)
  {
    int x, y, w, h;
    uint8_t i = this->find_free_slot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->icon_count)
    {
      icon = 0;
    }
    this->slots[i]->alarm = true;
    this->slots[i]->set_text(text, icon, w, this->duration);
    ESP_LOGD(TAG,"add_alarm icon: %d slot: %d text: %s",icon,i,text.c_str());
  }

  void EHMTX::add_screen(uint8_t icon, std::string text)
  {
    int x, y, w, h;
    uint8_t i = this->find_free_slot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->icon_count)
    {
      icon = 0;
    }
    this->slots[i]->set_text(text, icon, w, this->duration);
    ESP_LOGD(TAG,"add_screen icon: %d slot: %d text: %s",icon,i,text.c_str());
  }

  void EHMTX::set_brightness(uint8_t b)
  {
    this->brightness_ = (float) b/ (float)255;
    ESP_LOGI(TAG,"set_brightness %d => %3.0f %%",b, 100*this->brightness_);
    this->display->get_light()->set_correction(this->brightness_,this->brightness_,this->brightness_,this->brightness_); 
  }

  void EHMTX::add_screen_u(std::string iname, std::string text,uint16_t duration, bool alarm)
  {
    int x, y, w, h;
    uint8_t icon = this->find_icon(iname.c_str());
    if (icon >= this->icon_count)
    {
      icon = 0;
    }
    uint8_t i = this->find_free_slot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    this->slots[i]->set_text(text, icon, w, this->duration);   
    this->slots[i]->alarm = alarm;
    ESP_LOGD(TAG,"add_screen_u icon: %d iconname: %s slot: %d text: %s alarm: %d",icon,iname.c_str(),i,text.c_str(),alarm);
  }

  void EHMTX::add_screen_n(std::string iname, std::string text)
  {
    int x, y, w, h;
    uint8_t icon = this->find_icon(iname.c_str());
    if (icon >= this->icon_count)
    {
      icon = 0;
    }
    uint8_t i = this->find_free_slot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    this->slots[i]->set_text(text, icon, w, this->duration);
    ESP_LOGD(TAG,"add_screen_n icon: %d iconname: %s slot: %d text: %s",icon,iname.c_str(),i,text.c_str());
  }

  void EHMTX::add_screen_t(uint8_t icon, std::string text, uint16_t t)
  {
    int x, y, w, h;
    uint8_t i = this->find_free_slot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->icon_count)
    {
      icon = 0;
      ESP_LOGD(TAG,"icon no: %d not found",icon);
    }
    this->slots[i]->set_text(text, icon, w, t);
    ESP_LOGD(TAG,"add_screen_t icon: %d duration: %d slot: %d text: %s",icon,t,i,text.c_str());
  }

  void EHMTX::set_clock_time(uint16_t t)
  {
    this->clocktime = t;
  }

  void EHMTX::set_display(addressable_light::AddressableLightDisplay *disp)
  {
    this->display = disp;
  }

  void EHMTX::set_clock(time::RealTimeClock *clock)
  {
    this->clock = clock;
  }

  void EHMTX::draw_day_of_week()
  {
    auto dow = this->clock->now().day_of_week - 1;
    for (uint8_t i = 0; i <= 6; i++)
    {
      if (dow == i)
      {
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, this->text_color);
      }
      else
      {
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_cday);
      }
    }
  };

  void EHMTX::set_font_offset(int8_t x,int8_t y)
  {
    this->xoffset = x;
    this->yoffset = y;
  }

void EHMTX::dump_config() {
  ESP_LOGCONFIG(TAG, "EspHoMatriX %s",VERSION);
  ESP_LOGCONFIG(TAG, "Icons: %d of %d",this->icon_count,MAXICONS);
  ESP_LOGCONFIG(TAG, "Max screens: %d",MAXQUEUE);
  ESP_LOGCONFIG(TAG, "Intervall (ms) scroll: %d anim: %d",this->scroll_intervall,this->anim_intervall);
  ESP_LOGCONFIG(TAG, "Displaytime (s) clock: %d screen: %d",this->clocktime,this->screentime);
}

  void EHMTX::add_icon(display::Animation *icon, const char *name)
  {
    this->icons[this->icon_count] = icon;
    this->iconnames[this->icon_count] = name;
    this->icon_count++;
    ESP_LOGD(TAG,"add_icon no.: %d name: %s",this->icon_count,name);
  }

  void EHMTX::draw()
  {
    if (this->show_screen)
    {
      this->slots[this->active_slot]->draw();
    }
    else
    {
      this->draw_clock();
    }
    if (this->showindicator)
    {
      this->display->line(31, 5, 29, 7, this->indicator_color);
      this->display->draw_pixel_at(30, 7, this->indicator_color);
      this->display->draw_pixel_at(31, 6, this->indicator_color);
      this->display->draw_pixel_at(31, 7, this->indicator_color);
    }
  }
}