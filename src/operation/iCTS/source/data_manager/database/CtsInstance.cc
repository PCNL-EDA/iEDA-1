#include "CtsInstance.h"

namespace icts {

void CtsInstance::addPin(CtsPin *pin) {
  pin->set_instance(this);
  _pin_list.push_back(pin);
}

CtsPin *CtsInstance::get_in_pin() const { return find_pin(CtsPinType::kIn); }

CtsPin *CtsInstance::get_out_pin() const {
  CtsPin *found_pin = nullptr;
  for (auto *pin : _pin_list) {
    if (pin->is_io()) {
      if (pin->get_pin_type() == CtsPinType::kIn) {
        found_pin = pin;
        break;
      }
    } else {
      if (pin->get_pin_type() == CtsPinType::kOut) {
        found_pin = pin;
        break;
      }
    }
  }
  return found_pin;
  // return find_pin(CtsPinType::kOut);
}

CtsPin *CtsInstance::get_clk_pin() const {
  return find_pin(CtsPinType::kClock);
}

CtsPin *CtsInstance::find_pin(CtsPinType type) const {
  CtsPin *found_pin = nullptr;
  for (auto *pin : _pin_list) {
    if (pin->get_pin_type() == type) {
      found_pin = pin;
      break;
    }
  }
  return found_pin;
}

CtsPin *CtsInstance::get_load_pin() const {
  CtsPin *pin = get_clk_pin();
  return pin ? pin : get_in_pin();
}

}  // namespace icts