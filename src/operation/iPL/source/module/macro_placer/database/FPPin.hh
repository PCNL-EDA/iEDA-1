
#pragma once

#include "FPInst.hh"
namespace ipl::imp {
class FPNet;
class FPPin
{
 public:
  FPPin(){};
  ~FPPin(){};

  // setter
  void set_name(string name) { _name = name; }
  void set_x(int32_t x) { _coordinate->_x = x; }
  void set_y(int32_t y) { _coordinate->_y = y; }
  void set_instance(FPInst* instance) { _instance = instance; }
  void set_net(FPNet* net) { _net = net; }
  void set_io_pin() { _is_io_pin = true; }
  void set_weight(float weight) { _weight = weight; }

  // getter
  string get_name() const { return _name; }
  FPInst* get_instance() const { return _instance; }
  FPNet* get_net() const { return _net; }
  int32_t get_x()
  {
    if (!_is_io_pin) {
      return _instance->get_center_x() + get_offset_x();
    } else {
      return _coordinate->_x;
    }
  }
  int32_t get_y()
  {
    if (!_is_io_pin) {
      return _instance->get_center_y() + get_offset_y();
    } else {
      return _coordinate->_y;
    }
  }
  bool is_io_pin() { return _is_io_pin; }
  float get_weight() { return _weight; }

 private:
  int32_t get_offset_x();
  int32_t get_offset_y();

  string _name;
  bool _is_io_pin = false;
  Coordinate* _coordinate = new Coordinate();
  FPInst* _instance = nullptr;  // index of the macro in which the Pin is located
  FPNet* _net = nullptr;        // index of net to which Pin is attached
  float _weight = 1;            // instance's area is the weight
};

inline int32_t FPPin::get_offset_x()
{
  Orient orient = _instance->get_orient();
  if (orient == Orient::N || orient == Orient::FN) {
    return _coordinate->_x;
  } else if (orient == Orient::E || orient == Orient::FE) {
    return _coordinate->_y;
  } else if (orient == Orient::S || orient == Orient::FS) {
    return -(_coordinate->_x);
  } else if (orient == Orient::W || orient == Orient::FW) {
    return -(_coordinate->_y);
  }
  return 0;
};

inline int32_t FPPin::get_offset_y()
{
  Orient orient = _instance->get_orient();
  if (orient == Orient::N || orient == Orient::FN) {
    return _coordinate->_y;
  } else if (orient == Orient::E || orient == Orient::FW) {
    return -(_coordinate->_x);
  } else if (orient == Orient::S || orient == Orient::FS) {
    return -(_coordinate->_y);
  } else if (orient == Orient::W || orient == Orient::FE) {
    return _coordinate->_x;
  }
  return 0;
};
}  // namespace ipl::imp