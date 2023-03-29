#include "esphome.h"

namespace esphome
{

  EHMTX_screen::EHMTX_screen(EHMTX *config)
  {
    this->config_ = config;
    this->endtime = 0;
    this->centerx_ = 0;
    this->shiftx_ = 0;
    this->alarm = false;
  }

  bool EHMTX_screen::is_alarm() { return this->alarm; }

  bool EHMTX_screen::del_slot(uint8_t _icon)
  {
    if (this->icon == _icon)
    {
      this->endtime = 0;
      ESP_LOGD(TAG, "delete screen icon: %d", _icon);
      return true;
    }
    return false;
  }

  void EHMTX_screen::reset_shiftx()
  {
    this->shiftx_ = 0;
  }

  void EHMTX_screen::update_screen()
  {
    if (millis() - this->config_->last_scroll_time >= this->config_->scroll_interval && this->pixels_ > TEXTSTARTOFFSET)
    {
      this->shiftx_++;
      if (this->shiftx_ > this->pixels_ + TEXTSTARTOFFSET)
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
      extraoffset += 2;
    }

    if (!this->config_->icons[this->icon]->fullscreen)
    {
      if (this->alarm)
      {
        this->config_->display->print(this->centerx_ + TEXTSCROLLSTART - this->shiftx_ + extraoffset + this->config_->xoffset, this->config_->yoffset, this->config_->font, this->config_->alarm_color, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
      }
      else
      {
        this->config_->display->print(this->centerx_ + TEXTSCROLLSTART - this->shiftx_ + extraoffset + this->config_->xoffset, this->config_->yoffset, this->config_->font, this->text_color, esphome::display::TextAlign::BASELINE_LEFT,
                                      this->text.c_str());
      }
    }
    if (this->alarm)
    {
      this->config_->display->draw_pixel_at(30, 0, this->config_->alarm_color);
      this->config_->display->draw_pixel_at(31, 1, this->config_->alarm_color);
      this->config_->display->draw_pixel_at(31, 0, this->config_->alarm_color);
    }
  
    if (this->config_->show_gauge)
    {
      this->config_->draw_gauge();
      this->config_->display->image(2, 0, this->config_->icons[this->icon]);
      if (! this->config_->icons[this->icon]->fullscreen) {
        this->config_->display->line(10, 0, 10, 7, esphome::display::COLOR_OFF);
      }
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

  void EHMTX_screen::hold_slot(uint8_t _sec)
  {
    this->endtime += _sec;
    ESP_LOGD(TAG, "hold for %d secs", _sec);
  }

  void EHMTX_screen::set_text(std::string text, uint8_t icon, uint16_t pixel, uint16_t et,uint16_t show_time)
  {
    this->text = text;
    this->pixels_ = pixel;
    
    if (pixel < 23) {
      this->centerx_ = ceil((22-pixel)/2);
    }
    
    this->shiftx_ = 0;
    float display_duration = ceil((this->config_->scroll_count * (TEXTSTARTOFFSET + pixel) * this->config_->scroll_interval) / 1000);
    this->screen_time = (display_duration > show_time) ? display_duration : show_time;
    ESP_LOGD(TAG, "display length text: %s pixels %d calculated: %d show_time: %d default: %d", text.c_str(), pixel, this->screen_time, show_time, this->config_->screen_time);
    this->endtime = this->config_->clock->now().timestamp + et * 60;
    this->icon = icon;
  }

  void EHMTX_screen::set_text_color(uint8_t icon_id, Color text_color)
  {
    if (this->icon== icon_id ){
      this->text_color = text_color;
    }
  }
}
