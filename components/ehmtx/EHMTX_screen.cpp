#include "esphome.h"

namespace esphome
{

  EHMTX_screen::EHMTX_screen(EHMTX *config)
  {
    this->config = config;
    this->endtime = 0;
    this->alarm = false;
  }

  bool EHMTX_screen::isalarm() { return this->alarm; }

  bool EHMTX_screen::delslot(uint8_t _icon)
  {
    if (this->icon == _icon)
    {
      this->endtime = 0;
      this->icon = 0;
      return true;
    }
    return false;
  }

  void EHMTX_screen::update_screen()
  {
    if (millis() - this->config->lastscrolltime >= this->config->scrollintervall && this->pixels > (32 - 9))
    {
      this->shiftx++;
      if (this->shiftx > this->pixels + 2)
      {
        this->shiftx = 0;
      }
      this->config->lastscrolltime = millis();
    }
    if (millis() - this->config->lastanimtime >= this->config->animintervall &&
        (this->config->icons[this->icon]->get_current_frame() <
         this->config->icons[this->icon]->get_animation_frame_count()))
    {
      this->config->icons[this->icon]->next_frame();
      this->config->lastanimtime = millis();
    }
  }

  bool EHMTX_screen::active()
  {
    if (this->endtime > 0)
    {
      time_t ts = this->config->clock->now().timestamp;
      if (ts < this->endtime)
      {
        return true;
      }
    }
    return false;
  }

  void EHMTX_screen::_draw()
  {
    if (this->alarm)
    {
      this->config->display->print(TEXTSCROLLSTART - this->shiftx + this->config->xoffset, this->config->yoffset, this->config->font, this->config->alarmColor,
                                   this->text.c_str());
    }
    else
    {
      this->config->display->print(TEXTSCROLLSTART - this->shiftx + this->config->xoffset, this->config->yoffset, this->config->font, this->config->textColor,
                                   this->text.c_str());
    }
    this->config->display->print(TEXTSCROLLSTART - this->shiftx+ this->config->xoffset, this->config->yoffset, this->config->font, this->config->textColor,
                                 this->text.c_str());
    this->config->display->line(8, 0, 8, 7, esphome::display::COLOR_OFF);
    if (this->alarm)
    {
      this->config->display->draw_pixel_at(30, 0, this->config->alarmColor);
      this->config->display->draw_pixel_at(31, 1, this->config->alarmColor);
      this->config->display->draw_pixel_at(31, 0, this->config->alarmColor);
    }
    this->config->display->image(0, 0, this->config->icons[this->icon]);
  }

  void EHMTX_screen::draw()
  {
    this->_draw();
    this->update_screen();
  }

  void EHMTX_screen::setText(std::string text, uint8_t icon, uint8_t pixel, uint8_t et)
  {
    this->text = text;
    this->pixels = pixel;
    this->shiftx = 0;
    this->endtime = this->config->clock->now().timestamp + et * 60;
    if (this->alarm)
    {
      this->endtime += 2 * 60;
    }
    this->icon = icon;
  }

} // namespace esphome
