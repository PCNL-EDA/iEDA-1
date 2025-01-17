/**
 * @file Instance.h
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The class for instance in the netlist.
 * @version 0.1
 * @date 2021-02-03
 */

#pragma once

#include <algorithm>
#include <optional>
#include <utility>

#include "DesignObject.hh"
#include "DisallowCopyAssign.hh"
#include "Pin.hh"
#include "Vector.hh"
#include "liberty/Liberty.hh"
#include "string/Str.hh"

namespace ista {

class PinIterator;
class PinBusIterator;

/**
 * @brief class for instance of design.
 *
 */
class Instance : public DesignObject {
 public:
  Instance(const char* name, LibertyCell* cell_name);
  Instance(Instance&& other);
  Instance& operator=(Instance&& rhs);
  ~Instance() override = default;

  friend PinIterator;
  friend PinBusIterator;

  unsigned isInstance() override { return 1; }

  Pin* addPin(const char* name, LibertyPort* cell_port);
  Pin* getLastPin() { return _pins.back().get(); }
  std::optional<Pin*> getPin(const char* pin_name);
  LibertyCell* get_inst_cell() { return _inst_cell; }
  void set_inst_cell(LibertyCell* inst_cell) { _inst_cell = inst_cell; }

  Pin* findPin(LibertyPort* port);
  std::string getFullName() override { return get_name(); }

  void addPinBus(std::unique_ptr<PinBus> pin_bus) {
    _pin_buses.emplace_back(std::move(pin_bus));
  }
  PinBus* findPinBus(const std::string& bus_name) {
    auto* it = std::find_if(
        _pin_buses.begin(), _pin_buses.end(),
        [&bus_name](auto& pin_bus) { return bus_name == pin_bus->get_name(); });
    if (it != _pin_buses.end()) {
      return it->get();
    }
    return nullptr;
  }

 private:
  LibertyCell* _inst_cell;
  Vector<std::unique_ptr<Pin>> _pins;
  StrMap<Pin*> _str2pin;
  Vector<std::unique_ptr<PinBus>> _pin_buses;
  DISALLOW_COPY_AND_ASSIGN(Instance);
};

/**
 * @brief The class interator of pin.
 *
 */
class PinIterator {
 public:
  explicit PinIterator(Instance* inst);
  ~PinIterator() = default;

  bool hasNext();
  Pin* next();

 private:
  Instance* _inst;
  Vector<std::unique_ptr<Pin>>::iterator _iter;

  DISALLOW_COPY_AND_ASSIGN(PinIterator);
};

/**
 * @brief usage:
 * Instance* inst;
 * Pin* pin;
 * FOREACH_INSTANCE_PIN(inst, pin)
 * {
 *    do_something_for_pin();
 * }
 *
 */
#define FOREACH_INSTANCE_PIN(inst, pin) \
  for (ista::PinIterator iter(inst);    \
       iter.hasNext() ? pin = (iter.next()), true : false;)

/**
 * @brief The class iterator of pin bus.
 *
 */
class PinBusIterator {
 public:
  explicit PinBusIterator(Instance* inst);
  ~PinBusIterator() = default;

  bool hasNext();
  PinBus* next();

 private:
  Instance* _inst;
  Vector<std::unique_ptr<PinBus>>::iterator _iter;

  DISALLOW_COPY_AND_ASSIGN(PinBusIterator);
};

/**
 * @brief usage:
 * Instance* inst;
 * PinBus* pin_bus;
 * FOREACH_INSTANCE_PIN_BUS(inst, pin_bus)
 * {
 *    do_something_for_pinbus();
 * }
 *
 */
#define FOREACH_INSTANCE_PIN_BUS(inst, pin_bus) \
  for (ista::PinBusIterator iter(inst);         \
       iter.hasNext() ? pin_bus = (iter.next()), true : false;)

}  // namespace ista
