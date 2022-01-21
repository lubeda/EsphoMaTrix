#include "esphome.h"

namespace esphome
{

  EHMTX::EHMTX() : PollingComponent(TICKINTERVAL)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      this->slots[i] = new EHMTX_screen(this);
    }
    this->slots[0]->setText("start", 0, 32);
    this->showalarm = false;
    this->showscreen = false;
    this->showclock = true;
    this->activeslot = 0;
    this->lastslot = MAXQUEUE;
    this->nextactiontime = millis();
  }

  void EHMTX::drawclock()
  {
    if ((this->clock->now().timestamp - this->nextactiontime ) < this->clocktime) // alarm each 30000 secondsmillis() - this->lastclocktime < 3000)
    {
      this->display->strftime(6, this->fontoffset, this->font, "%H:%M",
                              this->clock->now());
    }
    else
    {
      this->display->strftime(5, this->fontoffset, this->font, "%d.%m.",
                              this->clock->now());
    }
    this->drawdayofweek();
  }

  void EHMTX::setup()
  {

  }

  bool EHMTX::findnextscreen(uint8_t *screen)
  {
    uint8_t count = 0;
    for (uint8_t screen = 0; screen < MAXQUEUE; screen++)
    {
      count++;
    }

    if ((count > 1) & (this->lastslot < MAXQUEUE) )
    {
        for (uint8_t screen = this->lastslot+1; screen < MAXQUEUE; screen++)
        {
          if (this->slots[screen]->active())
          { 
            return true;
          }
        }
    }

    for (uint8_t screen = 0; screen < MAXQUEUE; screen++)
      {
        if (this->slots[screen]->active() && screen != this->activeslot)
        {
          return true;
        }
      }
    screen = 0;
    return false;
  }

  void EHMTX::update() {
    if ((this->nextactiontime + 15) < this->clock->now().timestamp)
    {
      this->nextactiontime= this->clock->now().timestamp+3;
    }
  }

  void EHMTX::tick()
  {
    uint8_t slot = 0;

    if ((this->clock->now().timestamp - this->nextactiontime ) > this->screentime) // alarm each 30000 seconds
    {
      this->nextactiontime = this->clock->now().timestamp + this->screentime;
      ESP_LOGD("EHMTX","nextaction clock");
      this->showclock = true;
      this->showalarm = false;
      this->showscreen = false;
     
      if (this->findalarm(&slot))
      {
        ESP_LOGD("EHMTX","nextaction alarm");
        this->showclock = false;
        this->showalarm = true;
        this->activeslot = slot;
      }
      else
      {
        if (this->findnextscreen(&slot))
        {
          
          if (slot != this->lastslot)
          {
            ESP_LOGD("EHMTX","nextaction newscreen");
            this->showclock = false;
            this->activeslot = slot;
            this->lastslot = slot;
            this->showscreen = true;
          }
          else
          {          
            ESP_LOGD("EHMTX","nextaction screen => clock");
            this->lastslot = MAXQUEUE;
          }
        }
      }
      if (this->showalarm || this->showscreen)
      {
        this->slots[this->activeslot]->use();
      }
    }
    // ESP_LOGD("EHMTX","n %d ts %d diff %d st %d",this->nextactiontime,this->clock->now().timestamp,(this->nextactiontime - this->clock->now().timestamp), this->screentime);
  }

  // void update()
  // {
  //   this->showclock = true;
  //   ESP_LOGD("EHMTX", "Update start OK");
  //   /* jede Sekunde  */
  //   if (millis() - this->lastalarmtime >= 30000) // alarm each 30000 seconds
  //   {
  //     ESP_LOGD("EHMTX", "vor findalarm");
  //     uint8_t slot;
  //     if (this->findalarm(&slot))
  //     {
  //       this->activeslot = slot;
  //     }
  //     this->lastalarmtime = millis();
  //     this->showalarm = true;
  //     ESP_LOGD("EHMTX", "nach findalarm");
  //   }
  //   ESP_LOGD("EHMTX", "IF 2 ");
  //   if (not showalarm && (millis() - this->lastclocktime >= 60000)) // clock/date each minute
  //   {
  //     ESP_LOGD("EHMTX", "vor schowclock");

  //     this->lastclocktime = millis();
  //   }
  //   ESP_LOGD("EHMTX", "IF 3 ");
  //   if (millis() - this->lastscreentime >= this->screentime)
  //   {
  //     ESP_LOGD("EHMTX", "vor showscreen");
  //     for (uint8_t s = this->pointer; s < MAXQUEUE; s++)
  //     {
  //       ESP_LOGD("EHMTX", "in showscreen %d", s);
  //       if (this->slots[s]->active())
  //       {
  //         ESP_LOGD("EHMTX", "in showscreen active %d", s);
  //         this->showscreen = true;
  //         this->lastscreentime = millis();
  //         this->activeslot = s;
  //         this->pointer = s;
  //         break;
  //       }
  //     }
  //     this->pointer++;
  //     if (this->pointer == MAXQUEUE)
  //     {
  //       this->pointer = 0;
  //     }
  //   }
  //   ESP_LOGD("EHMTX", "Update C %d A %d S %d as %s", this->showclock, this->showalarm, this->showscreen, this->activeslot);
  // }

  void EHMTX::set_screentime(uint16_t t)
  {
    this->screentime = t;
  }

  void EHMTX::get_status(uint8_t icon)
  {
    ESP_LOGD("EHMTX", "getstatus activeslot %d", this->activeslot);
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {

      if (this->slots[i]->icon == icon)
      {

        ESP_LOGD("EHMTX", "getstatus slot %d icon %d text %s lifetime %d", i, this->slots[i]->icon, this->slots[i]->text.c_str(), this->slots[i]->lifetime);
      }
    }
  }

  bool EHMTX::findalarm(uint8_t *iconp)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->isalarm())
      {
        *iconp = i;
        return true;
      }
    }
    return false;
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
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->lifetime == 0)
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

  void EHMTX::set_lifetime(uint8_t lt)
  {
    this->lifetime = lt;
  }

  void EHMTX::del_screen(uint8_t icon)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->delslot(icon))
      {
        break;
      }
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
    this->slots[i]->setText(text, icon, w);
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
    this->slots[i]->setText(text, icon, w);
  }

  void EHMTX::set_clocktime(uint16_t t)
  {
    this->clocktime = t;
  }

  void EHMTX::set_iconlist(char *il)
  {
    this->iconlist = il;
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
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_ctoday);
      }
      else
      {
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_cday);
      }
    }
  };

  void EHMTX::set_fontoffset(int8_t t)
  {
    this->fontoffset = t;
  }

  void EHMTX::add_icon(display::Animation *icon)
  {
    this->icons[this->iconcount] = icon;
    this->iconcount++;
  }

  // void EHMTX::tick()
  // {
  //   // check what to display from update
  //   // display
  //   // check if to scroll oder nextframe
  //   if (this->showalarm)
  //   {
  //     this->slots[this->activeslot]->draw();
  //   }
  //   else if (this->showclock)
  //   {
  //     this->drawclock();
  //   }
  //   else if (this->showscreen)
  //   {
  //     this->slots[this->activeslot]->draw();
  //   }
  // }

  void EHMTX::draw()
  {
    if (this->showalarm | this->showscreen)
    {
      this->slots[this->activeslot]->draw();
    }
    else if (this->showclock)
    {
      this->drawclock();
    }
  }

  // void EHMTX::draw1()
  // {
  //   switch (this->pointer)
  //   {
  //   case 0:
  //     this->lastclocktime = millis();
  //     this->drawclock();
  //     break;
  //   case 1:
  //     uint8_t a;
  //     if (this->findalarm(&a))
  //     {
  //       this->slots[a]->use();
  //       this->slots[a]->draw();
  //       this->display->draw_pixel_at(30, 0, EHMTX_Calarm);
  //       this->display->draw_pixel_at(31, 1, EHMTX_Calarm);
  //       this->display->draw_pixel_at(31, 0, EHMTX_Calarm);
  //     }
  //     else
  //     {
  //       // skip
  //       this->pointer++;
  //       // EHMTX_draw();
  //     }
  //     break;

  //   default:

  //     if (this->slots[this->pointer - 3]->active())
  //     {
  //       this->slots[this->pointer - 3]->use();
  //       this->slots[this->pointer - 3]->draw();
  //     }
  //     break;
  //   }
  // }
}