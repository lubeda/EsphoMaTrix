#include "esphome.h"

namespace esphome
{
    
  EHMTX_Icon::EHMTX_Icon(const uint8_t *data_start, int width, int height, uint32_t animation_frame_count, display::ImageType type,std::string icon_name, bool revers, uint16_t frame_duration)  
    : Animation (data_start, width, height, animation_frame_count, type)
  {
      this->name= icon_name;
      this->reverse = revers;
      this->frame_duration = frame_duration;
      this->counting_up = true;
  }

  void EHMTX_Icon::next_frame(){
    // int get_animation_frame_count() const;
    // int get_current_frame() const;
    if (this->get_animation_frame_count() > 1)
    {
    //  this->current_frame_++;
    // if (this->current_frame_ >= animation_frame_count_) {
    //   this->current_frame_ = 0;
    //   }

      // 0 up =>  nf
      // l up
      if (this->counting_up)
      {
         if (this->reverse && (this->get_current_frame() == this->get_animation_frame_count()-2))
         {
           this->counting_up = false;
         } 
         Animation::next_frame();
       }
       else
       {
         if (this->get_current_frame() == 1 ) // this->get_animation_frame_count())
         {
           this->counting_up = true;
         }
         Animation::prev_frame();
       }
    }
  }

}
