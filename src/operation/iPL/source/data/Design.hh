/*
 * @Author: S.J Chen
 * @Date: 2022-01-26 20:20:49
 * @LastEditTime: 2022-11-21 22:32:59
 * @LastEditors: sjchanson 13560469332@163.com
 * @Description:
 * @FilePath: /iEDA/src/iPL/src/database/Design.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IPL_DATABASE_DESIGN_H
#define IPL_DATABASE_DESIGN_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Instance.hh"
#include "Net.hh"
#include "Pin.hh"
#include "Region.hh"

namespace ipl {

class Design {
 public:
  Design() = default;
  Design(const Design&) = delete;
  Design(Design&&) = delete;
  ~Design();

  Design& operator=(const Design&) = delete;
  Design& operator=(Design&&) = delete;

  // getter.
  std::string get_design_name() const { return _design_name; }
  std::vector<Instance*> get_instance_list() const { return _instance_list; }
  std::vector<Net*> get_net_list() const { return _net_list; }
  std::vector<Pin*> get_pin_list() const { return _pin_list; }
  std::vector<Region*> get_region_list() const { return _region_list; }

  // setter.
  void set_design_name(std::string design_name) {
    _design_name = std::move(design_name);
  }
  void add_instance(Instance* inst);
  void add_net(Net* net);
  void add_pin(Pin* pin);
  void add_region(Region* region);

  // function.
  Instance* find_instance(const std::string& inst_name) const;
  Net* find_net(const std::string& net_name) const;
  Pin* find_pin(const std::string& pin_name) const;
  Region* find_region(const std::string& region_name) const;
  void sortDataForParallel();

  // FOR DEBUG.
  void deleteInstForTest();

 private:
  std::string _design_name;
  std::vector<Instance*> _instance_list;
  std::vector<Net*> _net_list;
  std::vector<Pin*> _pin_list;
  std::vector<Region*> _region_list;

  std::map<std::string, Instance*> _name_to_inst_map;
  std::map<std::string, Net*> _name_to_net_map;
  std::map<std::string, Pin*> _name_to_pin_map;
  std::map<std::string, Region*> _name_to_region_map;
};

inline Design::~Design() {
  for (auto* inst : _instance_list) {
    delete inst;
  }
  _instance_list.clear();
  for (auto* net : _net_list) {
    delete net;
  }
  _net_list.clear();
  for (auto* pin : _pin_list) {
    delete pin;
  }
  _pin_list.clear();
  for (auto* region : _region_list) {
    delete region;
  }
  _region_list.clear();

  _name_to_inst_map.clear();
  _name_to_net_map.clear();
  _name_to_pin_map.clear();
  _name_to_region_map.clear();
}

inline void Design::add_instance(Instance* inst) {
  _instance_list.push_back(inst);
  _name_to_inst_map.emplace(inst->get_name(), inst);
}

inline void Design::add_net(Net* net) {
  _net_list.push_back(net);
  _name_to_net_map.emplace(net->get_name(), net);
}

inline void Design::add_pin(Pin* pin) {
  _pin_list.push_back(pin);
  _name_to_pin_map.emplace(pin->get_name(), pin);
}

inline void Design::add_region(Region* region) {
  _region_list.push_back(region);
  _name_to_region_map.emplace(region->get_name(), region);
}

inline Instance* Design::find_instance(const std::string& inst_name) const {
  Instance* inst = nullptr;
  auto inst_iter = _name_to_inst_map.find(inst_name);
  if (inst_iter != _name_to_inst_map.end()) {
    inst = (*inst_iter).second;
  }
  return inst;
}

inline Net* Design::find_net(const std::string& net_name) const {
  Net* net = nullptr;
  auto net_iter = _name_to_net_map.find(net_name);
  if (net_iter != _name_to_net_map.end()) {
    net = (*net_iter).second;
  }
  return net;
}

inline Pin* Design::find_pin(const std::string& pin_name) const {
  Pin* pin = nullptr;
  auto pin_iter = _name_to_pin_map.find(pin_name);
  if (pin_iter != _name_to_pin_map.end()) {
    pin = (*pin_iter).second;
  }
  return pin;
}

inline Region* Design::find_region(const std::string& region_name) const {
  Region* region = nullptr;
  auto region_iter = _name_to_region_map.find(region_name);
  if (region_iter != _name_to_region_map.end()) {
    region = (*region_iter).second;
  }
  return region;
}

inline void Design::sortDataForParallel() {
  // sort instance by area
  std::sort(_instance_list.begin(), _instance_list.end(),
            [](Instance* l_inst, Instance* r_inst) {
              return static_cast<int64_t>(l_inst->get_shape_height()) *
                         static_cast<int64_t>(l_inst->get_shape_width()) <
                     static_cast<int64_t>(r_inst->get_shape_height()) *
                         static_cast<int64_t>(r_inst->get_shape_width());
            });

  // sort net by degree
  std::sort(_net_list.begin(), _net_list.end(), [](Net* l_net, Net* r_net) {
    return l_net->get_pins().size() < r_net->get_pins().size();
  });
}

inline void Design::deleteInstForTest() {
  std::vector<Instance*> new_inst_list;

  for (auto* inst : _instance_list) {
    if (inst->get_pins().size() != 0) {
      new_inst_list.push_back(inst);
    }
  }

  _instance_list = new_inst_list;
}

}  // namespace ipl

#endif