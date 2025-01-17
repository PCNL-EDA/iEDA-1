#pragma once
/**
 * @project		iDB
 * @file		IdbNet.h
 * @date		25/05/2021
 * @version		0.1
 * @description


        Describe Nets information.

NETS numNets ;
  [– { netName
      [ ( {compName pinName | PIN pinName} [+ SYNTHESIZED] ) ] ...
    | MUSTJOIN ( compName pinName ) }
    [+ SHIELDNET shieldNetName ] ...
    [+ VPIN vpinName [LAYER layerName] pt pt
      [PLACED pt orient | FIXED pt orient | COVER pt orient] ] ...
    [+ SUBNET subnetName
      [ ( {compName pinName | PIN pinName | VPIN vpinName} ) ] ...
      [NONDEFAULTRULE rulename]
      [regularWiring] ...] ...
    [+ XTALK class]
    [+ NONDEFAULTRULE ruleName]
    [regularWiring] ...
    [+ SOURCE {DIST | NETLIST | TEST | TIMING | USER}]
    [+ FIXEDBUMP]
    [+ FREQUENCY frequency]
    [+ ORIGINAL netName]
    [+ USE {ANALOG | CLOCK | GROUND | POWER | RESET | SCAN | SIGNAL | TIEOFF}]
    [+ PATTERN {BALANCED | STEINER | TRUNK | WIREDLOGIC}]
    [+ ESTCAP wireCapacitance]
    [+ WEIGHT weight]
    [+ PROPERTY {propName propVal} ...] ...
  ;] ...
END NETS
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "../IdbEnum.h"
// #include "IdbInstance.h"
#include "../../../basic/geometry/IdbGeometry.h"
#include "../IdbObject.h"
#include "IdbPins.h"
#include "IdbRegularWire.h"

namespace idb {

class IdbInstanceList;
class IdnInstance;

class IdbNet : public IdbObject
{
 public:
  IdbNet();
  ~IdbNet();

  // getter
  const string get_net_name() { return _net_name; }
  const IdbConnectType get_connect_type() { return _connect_type; }
  bool is_signal() { return _connect_type == IdbConnectType::kSignal ? true : false; }
  bool is_clock() { return _connect_type == IdbConnectType::kClock ? true : false; }
  bool is_pdn() { return _connect_type == IdbConnectType::kPower || _connect_type == IdbConnectType::kGround ? true : false; }
  bool is_power() { return _connect_type == IdbConnectType::kPower ? true : false; }
  bool is_ground() { return _connect_type == IdbConnectType::kGround ? true : false; }
  const IdbInstanceType get_source_type() { return _source_type; }
  const int32_t get_weight() { return _weight; }
  const string get_original_net_name() const { return _original_net_name; }
  const int32_t get_xtalk() { return _xtalk; }
  const bool is_fix_bump() { return _fix_bump; }
  const double get_frequency() { return _frequency; }

  IdbCoordinate<int32_t>* get_average_coordinate() { return _average_coordinate; }

  IdbPin* get_io_pin() { return _io_pin; }
  IdbPins* get_instance_pin_list() { return _instance_pin_list; }
  int32_t get_pin_number()
  {
    int32_t number = _io_pin != nullptr ? 1 : 0;
    number = _instance_pin_list != nullptr ? number + _instance_pin_list->get_pin_num() : number;
    return number;
  }

  int32_t get_segment_num()
  {
    int number = 0;
    for (auto wire : _wire_list->get_wire_list()) {
      number += wire->get_num();
    }

    return number;
  }
  std::vector<IdbPin*> get_load_pins();  // tbd
  IdbInstanceList* get_instance_list() { return _instance_list; }
  IdbRegularWireList* get_wire_list() { return _wire_list; }

  IdbPin* get_driving_pin();

  // setter
  void set_net_name(string name)
  {
    assert(!name.empty());
    _net_name = name;
  }
  void set_connect_type(IdbConnectType type) { _connect_type = type; }
  void set_connect_type(string type);

  void set_source_type(string type);
  void set_weight(int32_t weight) { _weight = weight; }
  void set_original_net_name(string name) { _original_net_name = name; }
  void set_xtalk(int32_t xtalk) { _xtalk = xtalk; }
  void set_fix_bump(bool fix_bump) { _fix_bump = fix_bump; }
  void set_frequency(double frequency) { _frequency = frequency; }

  void set_average_coordinate(IdbCoordinate<int32_t>* average_coordinate) { _average_coordinate = average_coordinate; }

  // void add_io_pin(IdbPin* io_pin){_io_pin_list->add_pin_list(io_pin);}
  void set_io_pin(IdbPin* io_pin) { _io_pin = io_pin; }
  void add_instance_pin(IdbPin* inst_pin) { _instance_pin_list->add_pin_list(inst_pin); }
  bool set_bounding_box();

  // Delete
  void remove_pin(IdbPin* pin);

  // operator
  void clear_wire_list();
  bool checkConnection();
  int32_t wireLength();

 private:
  std::string _net_name;
  std::string _original_net_name;

  int32_t _weight;
  int32_t _xtalk;
  // NONDEFAULTRULE
  IdbInstanceType _source_type;
  IdbConnectType _connect_type;
  bool _fix_bump;
  double _frequency;
  IdbCoordinate<int32_t>* _average_coordinate;

  //------------------------tbd----------------
  // shiled Net
  // VPIN
  // SubNet
  // PATTERN
  // ESTCAP

  // PROPERTY

  // IdbPins* _io_pin_list;
  IdbPin* _io_pin;
  IdbPins* _instance_pin_list;
  IdbInstanceList* _instance_list;
  IdbRegularWireList* _wire_list;
};

class IdbNetList
{
 public:
  IdbNetList();
  ~IdbNetList();

  // getter
  std::vector<IdbNet*>& get_net_list() { return _net_list; }
  size_t get_num() { return _num; }
  size_t get_num_signal()
  {
    size_t number = 0;
    for (IdbNet* net : _net_list) {
      if (net->is_signal() || net->get_connect_type() == IdbConnectType::kNone) {
        number++;
      }
    }
    return number;
  }
  size_t get_num_clock()
  {
    size_t number = 0;
    for (IdbNet* net : _net_list) {
      if (net->is_clock()) {
        number++;
      }
    }
    return number;
  }

  size_t get_num_pdn()
  {
    size_t number = 0;
    for (IdbNet* net : _net_list) {
      if (net->is_pdn()) {
        number++;
      }
    }
    return number;
  }

  IdbNet* find_net(string name);
  IdbNet* find_net(size_t index);

  // setter
  void set_number(size_t number) { _num = number; }
  IdbNet* add_net(IdbNet* net = nullptr);
  IdbNet* add_net(string name, IdbConnectType type = IdbConnectType::kNone);
  bool remove_net(string name);

  void clear_wire_list();

  // operator
  void init(int32_t size) { _net_list.reserve(size); }
  bool checkConnection();
  uint64_t maxFanout();

 private:
  size_t _num;
  std::vector<IdbNet*> _net_list;
  std::unordered_map<string, IdbNet*> _net_map;
};

class IdbCheckNode
{
 public:
  IdbCheckNode();
  ~IdbCheckNode() = default;

 private:
  IdbCoordinate<int> _coordinate;
  int _layer_id;
};

class IdbNetChecker
{
 public:
  IdbNetChecker();
  ~IdbNetChecker() = default;

  /// fucntion
  bool checkNetConnection(IdbNet* net);

 private:
};

}  // namespace idb
