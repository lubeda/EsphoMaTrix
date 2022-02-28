#pragma once

#include "esphome.h"

namespace esphome {

class EHMTX;

class EhmtxSelect : public select::Select, public PollingComponent {
 public:

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }
  EHMTX *parent;

 protected:
  void control(const std::string &value) override;
};

}  
