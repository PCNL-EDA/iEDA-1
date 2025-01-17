#ifndef PIN_H
#define PIN_H

#include <string>

#include "CtsInstance.h"
#include "CtsNet.h"
#include "DesignObject.h"
#include "pgl.h"

namespace icts {
using std::string;

class CtsInstance;
class CtsNet;

enum class CtsPinType { kClock, kIn, kOut, kInOut, kOther };

class CtsPin : public DesignObject {
 public:
  CtsPin();
  explicit CtsPin(const string &pin_name);
  CtsPin(const string &pin_name, CtsPinType pin_type, CtsInstance *instance,
         CtsNet *net);
  CtsPin(const CtsPin &pin) = default;
  ~CtsPin() = default;

  // getter
  const string &get_pin_name() const { return _pin_name; }
  string get_full_name() const;
  CtsPinType get_pin_type() const { return _pin_type; }
  CtsInstance *get_instance() const { return _instance; }
  CtsNet *get_net() const { return _net; }
  bool is_io() { return _b_io; }

  // setter
  void set_pin_name(const string &pin_name) { _pin_name = pin_name; }
  void set_pin_type(CtsPinType pin_type) { _pin_type = pin_type; }
  void set_instance(CtsInstance *instance) { _instance = instance; }
  void set_net(CtsNet *net) { _net = net; }
  void set_io(bool b_io = false) { _b_io = b_io; }

 private:
  string _pin_name;
  CtsPinType _pin_type = CtsPinType::kOther;
  CtsInstance *_instance = nullptr;
  CtsNet *_net = nullptr;
  bool _b_io = false;
};

}  // namespace icts

#endif