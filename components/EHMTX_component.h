#include "esphome.h"
using namespace time;
using namespace display;

#define MAXQUEUE 12
#define TICKINTERVAL 100 // each 100ms
#define SCREENCYCLE 70   // max time per screen 7 seconds
#define CLOCKCYCLE 30
#define YFONTOFFSET -5

const Color EHMTX_Ctext = Color(178, 178, 191);
const Color EHMTX_Cwarn = Color(214, 214, 12);
const Color EHMTX_Calarm = Color(204, 51, 63);
const Color EHMTX_ctoday = Color(230, 230, 230);
const Color EHMTX_cday = Color(160, 160, 160);

// my stuff

uint16_t EHMTX_ticker = 0;
uint8_t EHMTX_icon = 0;
uint8_t EHMTX_pointer = 0;

void EHMTX_dayofweek()
{
  auto dow = id(rtctime).now().day_of_week - 1;
  for (uint8_t i = 0; i <= 6; i++)
  {
    if (dow == i)
    {
      EHMTX_display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_ctoday);
    }
    else
    {
      EHMTX_display->line(2 + i * 4, 7, i * 4 + 4, 7, EHMTX_cday);
    }
  }
};

class EHMTX_screen
{                    // The class
public:              // Access specifier
  uint16_t lifetime; // how often it will be displayed
  uint8_t icon;
  std::string text; // Attribute (string variable)
  int pixels;

  EHMTX_screen()
  {
    lifetime = 0;
    icon = 0;
  }

  void setText(std::string text, uint8_t icon = 0)
  {
    int x, y, w, h;
    this->text = text;
    EHMTX_display->get_text_bounds(0, 0, this->text.c_str(), EHMTX_font, TextAlign::LEFT, &x, &y, &w, &h);
    this->pixels = w;
    this->lifetime = 7 * SCREENCYCLE;
    if (icon < EHMTX_iconcount)
    {
      this->icon = icon;
    }
    else
    {
      this->icon = 0;
    }
  }

  void setAlarm(std::string text, uint8_t icon = 0)
  {
    this->setText(text, icon);
    this->lifetime = 15 * SCREENCYCLE;
  }

  bool active()
  {
    return (lifetime > 0);
  }

  void use()
  {
    if (this->lifetime > 0)
    {
      this->lifetime--;
    };
  }
};

//EHMTX_screen
EHMTX_screen *EHMTX_alarm = new EHMTX_screen();
EHMTX_screen *EHMTX_queue = new EHMTX_screen();
EHMTX_screen *EHMTX_slots[MAXQUEUE];

void EHMTX_delslot(uint8_t icon)
{
  for (uint8_t i = 0; i < MAXQUEUE; i++)
  {
    if (EHMTX_slots[i]->icon == icon)
    {
      EHMTX_slots[i]->lifetime = 0;
      EHMTX_slots[i]->icon = 0;
    }
  }
}

uint8_t EHMTX_slotfree(uint8_t icon)
{
  for (uint8_t i = 0; i < MAXQUEUE; i++)
  {
    if (EHMTX_slots[i]->icon == icon)
    {
      return i;
    }
  }

  for (uint8_t i = 0; i < MAXQUEUE; i++)
  {
    if (EHMTX_slots[i]->lifetime == 0)
    {
      return i;
    }
  }
  return 0;
}

// OK ??
void EHMTX_draw()
{
  switch (EHMTX_pointer)
  {
  case 0:
    EHMTX_display->print(1, YFONTOFFSET, EHMTX_font, EHMTX_Cwarn, "EHMTX");
    EHMTX_display->draw_pixel_at(30, 7, EHMTX_Cwarn);
    EHMTX_display->draw_pixel_at(31, 6, EHMTX_Cwarn);
    EHMTX_display->draw_pixel_at(31, 7, EHMTX_Cwarn);
    break;
  case 1:
    EHMTX_display->strftime(6, YFONTOFFSET, EHMTX_font, "%H:%M", id(rtctime).now());
    EHMTX_page="clock";
    EHMTX_dayofweek();
    break;
  case 2:
    EHMTX_display->strftime(5, YFONTOFFSET, EHMTX_font, "%d.%m.", id(rtctime).now());
    EHMTX_page="date";
    EHMTX_dayofweek();
    break;
  case 3:
    if (EHMTX_alarm->active())
    {
      uint8_t pixels = EHMTX_alarm->pixels;
      EHMTX_alarm->use();
      EHMTX_page= ("Alarm: " + EHMTX_alarm->text).c_str();
      if (pixels > 32-9) {
        EHMTX_display->print(9 - EHMTX_ticker % pixels, YFONTOFFSET, EHMTX_font, EHMTX_Cwarn, EHMTX_alarm->text.c_str());
      } else
      {
        EHMTX_display->print(9, YFONTOFFSET, EHMTX_font, EHMTX_Cwarn, EHMTX_alarm->text.c_str());
      }
      EHMTX_display->line(8, 0, 8, 7, COLOR_OFF);
      EHMTX_display->draw_pixel_at(30, 0, EHMTX_Calarm);
      EHMTX_display->draw_pixel_at(31, 1, EHMTX_Calarm);
      EHMTX_display->draw_pixel_at(31, 0, EHMTX_Calarm);
      EHMTX_display->image(0, 0, EHMTX_icons[EHMTX_alarm->icon]);
      if (EHMTX_ticker % 4 == 0)
      {
        if (EHMTX_icons[EHMTX_alarm->icon]->get_current_frame() < EHMTX_icons[0]->get_animation_frame_count())
        {
          EHMTX_icons[EHMTX_alarm->icon]->next_frame();
        }
      }
    }
    else
    {
      // skip
      EHMTX_pointer++;
      // EHMTX_draw();
    }
    break;

  default:
    
    if (EHMTX_slots[EHMTX_pointer - 4]->active())
    {
    
      std::string text = EHMTX_slots[EHMTX_pointer - 4]->text;
      EHMTX_page= ("Screen: " + text).c_str();
      uint8_t icon = EHMTX_slots[EHMTX_pointer - 4]->icon;
      uint8_t pixels = EHMTX_slots[EHMTX_pointer - 4]->pixels;
      EHMTX_slots[EHMTX_pointer - 4]->use();
      if (pixels > 32-9) {
        EHMTX_display->print(9 - EHMTX_ticker % pixels, YFONTOFFSET, EHMTX_font, EHMTX_Ctext,text.c_str());
      } else
      {
        EHMTX_display->print(9, YFONTOFFSET, EHMTX_font, EHMTX_Ctext, text.c_str());
      }    
      EHMTX_display->line(8, 0, 8, 7, COLOR_OFF);
      EHMTX_display->image(0, 0, EHMTX_icons[icon]);
      if (EHMTX_ticker % 4 == 0)
        EHMTX_icons[icon]->next_frame();
    }
    else
    {
      EHMTX_pointer++;
      if (EHMTX_pointer > MAXQUEUE + 3)
      {
        EHMTX_pointer = 1; // skip to clock
      }
      EHMTX_draw();
    }
    break;
  }
}

class EHMTX_Component : public PollingComponent
{

public:
  EHMTX_Component() : PollingComponent(TICKINTERVAL) {}

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  void setup() override
  {
    for (uint8_t i = 0; i < MAXQUEUE; i++)
    {
      EHMTX_slots[i] = new EHMTX_screen();
    }
  }

  // count EHMTX_ticker to SCREENCYCLE then EHMTX_pointer++ and restart
  // if EHMTX_pointer >= MAXQUEUE+3 EHMTX_pointer=1
  // EHMTX_pointer = 0 => bootscreen
  // EHMTX_pointer = 1 => clock
  // EHMTX_pointer = 2 => date
  // EHMTX_pointer = 3 => alarm
  // EHMTX_pointer > 3 <> MAXQUEUE+3 => queue[EHMTX_pointer]

  void update() override
  {
    EHMTX_ticker++;

    if ((EHMTX_pointer == 1) || (EHMTX_pointer == 2))
    {
      if (EHMTX_ticker >= CLOCKCYCLE)
      {
        EHMTX_ticker = 0;
        EHMTX_pointer++;
      }
    }
    else if (EHMTX_ticker >= SCREENCYCLE)
    {
      EHMTX_ticker = 0;
      if ((EHMTX_pointer < MAXQUEUE + 3) & (EHMTX_pointer > 2))
      {
        EHMTX_pointer++;
      }
      else
      {
        EHMTX_pointer = 1; // skip boot
      }
    }
  }
};
