#include "esphome.h"

namespace esphome
{

  EHMTX_screen::EHMTX_screen(EHMTX *config)
  {
    this->config_ = config;
    this->endtime = 0;
    this->alarm = false;
  }

  bool EHMTX_screen::is_alarm() { return this->alarm; }

  bool EHMTX_screen::del_slot(uint8_t _icon)
  {
    if (this->icon == _icon)
    {
      this->endtime = 0;
      this->icon = 0;
      ESP_LOGD(TAG, "delete screen icon: %d", _icon);
      return true;
    }
    return false;
  }

  void EHMTX_screen::update_screen()
  {
    if (millis() - this->config_->last_scroll_time >= this->config_->scroll_intervall && this->pixels_ > TEXTSTARTOFFSET )
    {
      this->shiftx_++;
      if (this->shiftx_ > this->pixels_ + TEXTSTARTOFFSET )
      {
        this->shiftx_ = 0;
      }
      this->config_->last_scroll_time = millis();
    }
    if (millis() - this->config_->last_anim_time >= this->config_->icons[this->icon]->frame_duration)
    {
      this->config_->icons[this->icon]->next_frame();
      this->config_->last_anim_time = millis();
    }
  }

  bool EHMTX_screen::active()
  {
    if (this->endtime > 0)
    {
      time_t ts = this->config_->clock->now().timestamp;
      if (ts < this->endtime)
      {
        return true;
      }
    }
    return false;
  }

  void EHMTX_screen::draw_()
  {
    int8_t extraoffset = 0;

    if (this->pixels_ > TEXTSTARTOFFSET)
    {
      extraoffset = TEXTSTARTOFFSET;
    }
    if (this->config_->show_gauge)
    {
      extraoffset +=2;
    }
     
    if (this->alarm)
    {
      this->config_->display->print(TEXTSCROLLSTART - this->shiftx_ + extraoffset + this->config_->xoffset, this->config_->yoffset, this->config_->font, this->config_->alarm_color, esphome::display::TextAlign::BASELINE_LEFT,
                                    this->text.c_str());
    }
    else
    {
      this->config_->display->print(TEXTSCROLLSTART - this->shiftx_ + extraoffset + this->config_->xoffset, this->config_->yoffset, this->config_->font, this->config_->text_color, esphome::display::TextAlign::BASELINE_LEFT,
                                    this->text.c_str());
    }
    
    if (this->alarm)
    {
      this->config_->display->draw_pixel_at(30, 0, this->config_->alarm_color);
      this->config_->display->draw_pixel_at(31, 1, this->config_->alarm_color);
      this->config_->display->draw_pixel_at(31, 0, this->config_->alarm_color);
    }
  
    if (this->config_->show_gauge)
    {
      this->config_->display->line(0, 7, 0, 0,esphome::display::COLOR_OFF );
      this->config_->display->line(0, 7, 0, this->config_->gauge_value, this->config_->gauge_color);
      this->config_->display->line(1, 7, 1, 0,esphome::display::COLOR_OFF );
      this->config_->display->image(2, 0, this->config_->icons[this->icon]);
      this->config_->display->line(10, 0, 10, 7, esphome::display::COLOR_OFF);
    }
    else
    {
        this->config_->display->line(8, 0, 8, 7, esphome::display::COLOR_OFF);
        this->config_->display->image(0, 0, this->config_->icons[this->icon]);
    }

  }

  void EHMTX_screen::draw()
  {
    this->draw_();
    this->update_screen();
  }

  void EHMTX_screen::set_text(std::string text, uint8_t icon, uint8_t pixel, uint16_t et)
  {
    this->text = text;
    this->pixels_ = pixel;
    this->shiftx_ = 0;
    float dd = (2 * (TEXTSTARTOFFSET + pixel) * this->config_->scroll_intervall) / 1000;
    this->display_duration = (dd > this->config_->screen_time) ? dd : this->config_->screen_time;
    ESP_LOGD(TAG, "display length text: %s t: %.2f default: %d", text.c_str(), this->display_duration, this->config_->screen_time);
    this->endtime = this->config_->clock->now().timestamp + et * 60;
     this->icon = icon;
  }
}
