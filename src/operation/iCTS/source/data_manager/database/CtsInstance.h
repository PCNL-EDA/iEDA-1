#pragma once

#include <cassert>
#include <string>
#include <vector>

#include "CtsPin.h"
#include "DesignObject.h"
#include "pgl.h"

namespace icts {
class CtsPin;
enum class CtsPinType;
using std::string;
using std::vector;

enum class CtsInstanceType {
  kBuffer,        // up level buffer
  kSink,          // flip flop
  kSteinerPoint,  // steiner point
  kClockGate,     // clock gate
  kBufferSink,
  kLogic
};

class CtsInstance : public DesignObject {
 public:
  CtsInstance() = default;
  CtsInstance(const string &name) : _name(name) {}
  CtsInstance(const string &name, const string &cell_master,
              CtsInstanceType type, Point location)
      : _name(name),
        _cell_master(cell_master),
        _type(type),
        _location(location) {}

  // getter
  const string &get_name() { return _name; }
  string get_cell_master() const { return _cell_master; }
  CtsInstanceType get_type() const { return _type; }
  Point get_location() const { return _location; }
  vector<CtsPin *> &get_pin_list() { return _pin_list; }
  CtsPin *get_in_pin() const;
  CtsPin *get_out_pin() const;
  CtsPin *get_clk_pin() const;
  CtsPin *get_load_pin() const;
  int get_level() const { return _level; }
  bool is_virtual() { return _b_virtual; }
  int getSubWirelength() const { return _sub_wirelength; }

  // setter
  void set_name(const string &name) { _name = name; }
  void set_cell_master(const string &cell_master) {
    _cell_master = cell_master;
  }
  void set_type(CtsInstanceType type) { _type = type; }
  void set_location(const Point &location) { _location = location; }
  void set_location(int x, int y);
  void set_virtual(bool b_virtual) { _b_virtual = b_virtual; }
  void set_level(const int &level) { _level = level; }
  void setSubWirelength(const int &sub_wirelength) {
    _sub_wirelength = sub_wirelength;
  }
  void addPin(CtsPin *pin);

 private:
  CtsPin *find_pin(CtsPinType type) const;

 private:
  string _name;
  string _cell_master;
  vector<CtsPin *> _pin_list;
  CtsInstanceType _type = CtsInstanceType::kSink;
  Point _location;
  int _level = 0;
  bool _b_virtual = false;
  int _sub_wirelength = 0;
};

inline void CtsInstance::set_location(int x, int y) {
  _location.x(x);
  _location.y(y);
}

}  // namespace icts
