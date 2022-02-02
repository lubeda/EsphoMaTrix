#include "esphome.h"

namespace esphome
{

  EHMTX::EHMTX() : PollingComponent(TICKINTERVAL)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      this->slots[i] = new EHMTX_screen(this);
    }
    this->showscreen = false;
    this->activeslot = 0;
    this->textColor = Color(200, 200, 200);
    this->alarmColor = Color(200, 50, 50);
    this->lastclocktime = 0;
  }

  void EHMTX::set_indicatorcolor(int r, int g, int b)
  {
    this->indicatorColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  uint8_t EHMTX::find_icon(std::string name)
  {
    for (uint8_t i = 0; i < this->iconcount; i++)
    { 
      if (strcmp (this->iconnames[i],name.c_str()) == 0){
        return i;
      }
    }
    return MAXICONS;
  }


  void EHMTX::set_alarmcolor(int r, int g, int b)
  {
    this->alarmColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  void EHMTX::set_textcolor(int r, int g, int b)
  {
    this->textColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
  }

  void EHMTX::set_indicatoroff()
  {
    this->showindicator = false;
  }
  void EHMTX::set_indicatoron()
  {
    this->showindicator = true;
  }

  void EHMTX::drawclock()
  {
    if ((this->clock->now().timestamp - this->nextactiontime) < this->clocktime) //
    {
      this->display->strftime(6+this->xoffset, this->yoffset, this->font, this->textColor, "%H:%M",
                              this->clock->now());
    }
    else
    {
      this->display->strftime(5+this->xoffset, this->yoffset, this->font, this->textColor, "%d.%m.",
                              this->clock->now());
    }
    this->drawdayofweek();
  }

  void EHMTX::setup()
  {
  }

  uint8_t EHMTX::countscreens()
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

  uint8_t EHMTX::findnextscreen()
  {
    uint8_t screen = MAXQUEUE;

    if (this->countscreens()== 1){
      for (screen = 0; screen < MAXQUEUE; screen++)
      {
        if (this->slots[screen]->active())
        {
          return screen;
        }
      }  
    }

    for (screen = (this->activeslot + 1); screen < MAXQUEUE; screen++)
    {
      if (this->slots[screen]->active())
      {
        return screen;
      }
    }
    for (screen = 0; screen < this->activeslot; screen++)
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
    if ((this->nextactiontime + 15) < ts)
    {
      this->nextactiontime = ts + 3;
      this->lastclocktime = ts;
    }
  }
  
  void EHMTX::tick()
  {
    
    time_t ts = this->clock->now().timestamp;
    
    if ((ts - this->nextactiontime) > this->screentime) // alarm each 30000 seconds
    {
      
      this->nextactiontime = ts + this->screentime;

      this->showscreen = false;

      if (!(ts - this->lastclocktime > 60))
      {
        if (this->countscreens() > 0)
        {
          
          uint8_t fns = this->findnextscreen();
          if (fns < MAXQUEUE)
          {
            this->activeslot = fns;
            this->showscreen = true;
          }
        }
      }
      if (this->showscreen == false)
      {
        this->lastclocktime = this->clock->now().timestamp;
      }
    }
  }

  void EHMTX::set_screentime(uint16_t t)
  {
    this->screentime = t;
  }

  void EHMTX::set_duration(uint8_t t)
  {
    this->duration = t;
  }

  void EHMTX::get_status()
  {
    uint8_t status = 0;
    time_t ts = this->clock->now().timestamp;
    status = this->showscreen ? 1 : 0;
    ESP_LOGI("EHMTX", "status status: %d  as: %d", status, this->activeslot);
    ESP_LOGI("EHMTX", "status screen count: %d", this->countscreens());
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->active())
      {
        int td = (int) this->slots[i]->endtime - ts;
        ESP_LOGI("EHMTX", "status slot: %d icon: %d  text: %s end: %d", i, this->slots[i]->icon, this->slots[i]->text.c_str(), td);
      }
    }
    for (uint8_t i = 0; i < this->iconcount; i++)
    {
      ESP_LOGI("EHMTX", "status icon: %d name: %s", i, this->iconnames[i]);
    }
  }

  uint8_t EHMTX::findfreeslot(uint8_t icon)
  {
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

  void EHMTX::set_animintervall(uint16_t ai)
  {
    this->animintervall = ai;
  }

  void EHMTX::set_scrollintervall(uint16_t si)
  {
    this->scrollintervall = si;
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
      this->slots[i]->delslot(icon);
    }
  }

  void EHMTX::add_alarm(uint8_t icon, std::string text)
  {
    int x, y, w, h;
    uint8_t i = this->findfreeslot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->iconcount)
    {
      icon = 0;
    }
    this->slots[i]->alarm = true;
    this->slots[i]->setText(text, icon, w, this->duration);
  }

  void EHMTX::add_screen(uint8_t icon, std::string text)
  {
    int x, y, w, h;
    uint8_t i = this->findfreeslot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->iconcount)
    {
      icon = 0;
    }
    this->slots[i]->setText(text, icon, w, this->duration);
  }

  void EHMTX::add_screen_n(std::string iname, std::string text)
  {
    int x, y, w, h;
    uint8_t icon = this->find_icon(iname.c_str());
    if (icon >= this->iconcount)
    {
      icon = 0;
    }
    uint8_t i = this->findfreeslot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    this->slots[i]->setText(text, icon, w, this->duration);
  }

  void EHMTX::add_screen_t(uint8_t icon, std::string text, uint8_t t)
  {
    int x, y, w, h;
    uint8_t i = this->findfreeslot(icon);
    this->display->get_text_bounds(0, 0, text.c_str(), this->font, display::TextAlign::LEFT, &x, &y, &w, &h);
    if (icon >= this->iconcount)
    {
      icon = 0;
    }
    this->slots[i]->setText(text, icon, w, t);
  }

  void EHMTX::set_clocktime(uint16_t t)
  {
    this->clocktime = t;
  }

  void EHMTX::set_display(display::DisplayBuffer *disp)
  {
    this->display = disp;
  }

  void EHMTX::set_clock(time::RealTimeClock *clock)
  {
    this->clock = clock;
  }

  void EHMTX::drawdayofweek()
  {
    auto dow = this->clock->now().day_of_week - 1;
    for (uint8_t i = 0; i <= 6; i++)
    {
      if (dow == i)
      {
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, this->textColor);
      }
      else
      {
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_cday);
      }
    }
  };

  void EHMTX::set_fontoffset(int8_t x,int8_t y)
  {
    this->xoffset = x;
    this->yoffset = y;
  }

  void EHMTX::add_icon(display::Animation *icon, const char *name)
  {
    this->icons[this->iconcount] = icon;
    this->iconnames[this->iconcount] = name;
    this->iconcount++;
  }

  void EHMTX::draw()
  {
    if (this->showscreen)
    {
      this->slots[this->activeslot]->draw();
    }
    else
    {
      this->drawclock();
    }
    if (this->showindicator)
    {
      this->display->line(31, 5, 29, 7, this->indicatorColor);
      this->display->draw_pixel_at(30, 7, this->indicatorColor);
      this->display->draw_pixel_at(31, 6, this->indicatorColor);
      this->display->draw_pixel_at(31, 7, this->indicatorColor);
    }
  }
}