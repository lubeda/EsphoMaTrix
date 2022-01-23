#include "esphome.h"

namespace esphome
{

  EHMTX_screen::EHMTX_screen(EHMTX *config) { this->config = config; }

  bool EHMTX_screen::isalarm() { return this->alarm; }

  bool EHMTX_screen::delslot(uint8_t _icon)
  {
    if (this->icon == _icon)
    {
      this->lifetime = 0;
      this->icon = 0;
      return true;
    }
    return false;
  }

  void EHMTX_screen::update_screen()
  {
    bool screendirty = false;
    if (millis() - this->config->lastscrolltime >= this->config->scrollintervall && this->pixels > (32 - 9))
    {
      this->shiftx++;
      if (this->shiftx > this->pixels + 2)
      {
        this->shiftx = 0;
      }
      screendirty = true;
      this->config->lastscrolltime = millis();
    }
    if (millis() - this->config->lastanimtime >= this->config->animintervall &&
        (this->config->icons[this->icon]->get_current_frame() <
         this->config->icons[this->icon]->get_animation_frame_count()))
    {
      this->config->icons[this->icon]->next_frame();
      this->config->lastanimtime = millis();
      screendirty = true;
    }
    if (screendirty)
    {
      this->_draw();
    }
  }

  void EHMTX_screen::use()
  {
    if (this->lifetime > 0)
    {
      this->lifetime--;
    }
    else
    {
      this->alarm = false;
    }
  }

  bool EHMTX_screen::active()
  {
    if (this->lifetime > 0)
    {
      return true;
    }
    return false;
  }

  void EHMTX_screen::_draw()
  {
    if (this->alarm)
    {
      this->config->display->print(9 - this->shiftx, this->config->fontoffset, this->config->font, this->config->alarmColor,
                                   this->text.c_str());
    }
    else
    {
      this->config->display->print(9 - this->shiftx, this->config->fontoffset, this->config->font, this->config->textColor,
                                   this->text.c_str());
    }
    this->config->display->print(9 - this->shiftx, this->config->fontoffset, this->config->font, this->config->textColor,
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
    if (this->active())
    {
      this->_draw();
      this->update_screen();
    }
  }

  void EHMTX_screen::setText(std::string text, uint8_t icon, uint8_t pixel)
  {
    this->text = text;
    this->pixels = pixel;
    this->shiftx = 0;
    this->lifetime = this->config->lifetime;
    if (this->alarm)
    {
      this->lifetime += 4;
    }
    this->icon = icon;
  }

} // namespace esphome
