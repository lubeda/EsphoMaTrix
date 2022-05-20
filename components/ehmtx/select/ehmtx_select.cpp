#include "ehmtx_select.h"
#include "esphome/core/log.h"

namespace esphome {

static const char *const TAG = "ehmtx.select";

void EhmtxSelect::setup() {
   this->publish_state("initializing...");
}

void EhmtxSelect::update() {
  if (this->parent != NULL) {
    std::string value;
    value = this->parent->get_current();
    this->publish_state(value);
  }
}

void EhmtxSelect::dump_config() {
  LOG_SELECT(TAG," ", this);
  LOG_UPDATE_INTERVAL(this);
}

void EhmtxSelect::control(const std::string &value) {
  // value from HA => check if displayable
  if (this->parent != NULL) {
    ESP_LOGD(TAG, "select control to: %s",value.c_str());
    this->parent->force_screen(value.c_str());
  } 
}

}  // namespace esphome
