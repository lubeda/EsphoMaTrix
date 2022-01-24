#include "esphome.h"

namespace esphome
{

  EHMTX::EHMTX() : PollingComponent(TICKINTERVAL)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      this->slots[i] = new EHMTX_screen(this);
    }
    this->slots[0]->setText("EHMTX", 0, 32);
    this->slots[0]->alarm = false;
    this->showalarm = false;
    this->showscreen = false;
    this->showclock = true;
    this->activeslot = 1;
    this->pointer = 0;
    this->lastslot = MAXQUEUE;
    this->textColor = Color(200, 200, 200);
    this->alarmColor = Color(200, 50, 50);
    this->nextactiontime = millis();
  }

  void EHMTX::set_indicatorcolor(int r, int g, int b)
  {
    this->indicatorColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGI("EHMTX", "Indicator color r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_alarmcolor(int r, int g, int b)
  {
    this->alarmColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGI("EHMTX", "Alarm color r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_textcolor(int r, int g, int b)
  {
    this->textColor = Color((uint8_t)r & 248, (uint8_t)g & 252, (uint8_t)b & 248);
    ESP_LOGI("EHMTX", "Text color r: %d g: %d b: %d", r, g, b);
  }

  void EHMTX::set_indicatoroff()
  {
    this->showindicator = false;
    ESP_LOGI("EHMTX", "Indicator off");
  }
  void EHMTX::set_indicatoron()
  {
    this->showindicator = true;
    ESP_LOGI("EHMTX", "Indicator on");
  }

  void EHMTX::drawclock()
  {
    if ((this->clock->now().timestamp - this->nextactiontime) < this->clocktime) //
    {
      this->display->strftime(6, this->fontoffset, this->font, this->textColor, "%H:%M",
                              this->clock->now());
    }
    else
    {
      this->display->strftime(5, this->fontoffset, this->font, this->textColor, "%d.%m.",
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
    if ((this->nextactiontime + 15) < this->clock->now().timestamp)
    {
      this->nextactiontime = this->clock->now().timestamp + 3;
    }
  }

  void EHMTX::tick()
  {
    uint8_t slot = 0;
    uint8_t count;

    if (this->pointer > 0)
    {
      this->pointer--;
    }
    else
    {
      this->pointer = MAXQUEUE - 1;
    }

    if ((this->clock->now().timestamp - this->nextactiontime) > this->screentime) // alarm each 30000 seconds
    {
      this->nextactiontime = this->clock->now().timestamp + this->screentime;
      count = this->countscreens();

      if (count < 4)
      {
        ESP_LOGD("EHMTX", "nextaction clock CS %d", count);
        this->showclock = true;
        this->showalarm = false;
        this->showscreen = false;
      }

      if ((this->pointer % 5) != 0)
      {
        if ((this->findalarm() < MAXQUEUE))
        {
          ESP_LOGD("EHMTX", "nextaction alarm");
          this->showclock = false;
          this->showalarm = true;
          this->activeslot = this->findalarm();
        }
        else
        {
          if ((this->pointer % 6) != 0)
          {
            slot = this->findnextscreen();
            if (slot < MAXQUEUE)
            {
              ESP_LOGD("EHMTX", "nextaction newscreen");
              this->showclock = false;
              this->activeslot = slot;
              this->lastslot = slot;
              this->showscreen = true;
            }
          }
        }
      }
      if (this->showalarm || this->showscreen)
      {
        this->slots[this->activeslot]->use();
      }
    }
  }

  void EHMTX::set_screentime(uint16_t t)
  {
    this->screentime = t;
  }

  void EHMTX::get_status()
  {
    uint8_t status = 0;
    status += this->showclock ? 1 : 0;
    status += this->showscreen ? 2 : 0;
    status += this->showalarm ? 4 : 0;
    ESP_LOGI("EHMTX", "getstatus status: %d  ls: % d as: %d p:%d", status, this->lastslot, this->activeslot, this->pointer);
    ESP_LOGI("EHMTX", "getstatus screen count: %d", this->countscreens());
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->active())
      {
        ESP_LOGI("EHMTX", "getstatus slot: %d icon: %d  lt: %d text: %s", i, this->slots[i]->icon, this->slots[i]->lifetime, this->slots[i]->text.c_str());
      }
    }
    ESP_LOGI("EHMTX", "getstatus %d iconnames: %s", this->iconcount, this->iconlist.c_str());
  }

  uint8_t EHMTX::findalarm()
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->isalarm())
      {
        return i;
      }
    }
    return MAXQUEUE;
  }

  uint8_t EHMTX::findfreeslot(uint8_t icon)
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      if (this->slots[i]->icon == icon)
      {
        ESP_LOGI("EHMTX", "upd screen no. %d", i);
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
        ESP_LOGI("EHMTX", "del screen no. %d", i);
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
    ESP_LOGI("EHMTX", "new alarm no. %d ic: %d t:%s", i, icon, text.c_str());
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
    ESP_LOGI("EHMTX", "new screen no. %d ic: %d t:%s", i, icon, text.c_str());
  }

  void EHMTX::set_clocktime(uint16_t t)
  {
    this->clocktime = t;
  }

  void EHMTX::set_iconlist(std::string il)
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
        this->display->line(2 + i * 4, 7, i * 4 + 4, 7, this->textColor);
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
    if (this->showindicator)
    {
      this->display->line(31, 5, 29, 7, this->indicatorColor);
      this->display->draw_pixel_at(30, 7, this->indicatorColor);
      this->display->draw_pixel_at(31, 6, this->indicatorColor);
      this->display->draw_pixel_at(31, 7, this->indicatorColor);
    }
  }
}