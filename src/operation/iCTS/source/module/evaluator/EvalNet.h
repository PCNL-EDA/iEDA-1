#pragma once

#include <unordered_map>
#include <utility>

#include "CTSAPI.hpp"
#include "CtsConfig.h"
#include "CtsInstance.h"
#include "CtsNet.h"
#include "log/Log.hh"

namespace icts {
using ieda::Log;
using std::vector;

enum NetType
{
  kTop = 0,
  kTrunk = 1,
  kLeaf = 2,
};

class EvalNet
{
 public:
  EvalNet(CtsNet* net) : _net(net)
  {
    auto insts = net->get_instances();
    for (auto* inst : insts) {
      _name_to_inst.insert(std::make_pair(inst->get_name(), inst));
    }
  }
  EvalNet(const EvalNet& net) = default;
  ~EvalNet() = default;
  bool is_newly() const { return _net->is_newly(); }
  std::string get_name() const { return _net->get_net_name(); }
  CtsNet* get_clk_net() const { return _net; }
  CtsInstance* get_driver() const { return get_clk_net()->get_driver_inst(); }
  vector<CtsSignalWire> get_signal_wires() const { return _net->get_signal_wires(); }
  CtsInstance* get_instance(const std::string& inst_name) const
  {
    if (_name_to_inst.count(inst_name) == 0) {
      return nullptr;
    } else {
      return _name_to_inst.at(inst_name);
    }
  }
  vector<CtsInstance*> get_instances() const { return _net->get_instances(); }
  vector<CtsPin*> get_pins() const { return _net->get_pins(); }
  CtsPin* get_driver_pin() const { return _net->get_driver_pin(); }
  Point getCenterPoint() const
  {
    LOG_FATAL_IF(_net->get_pins().empty()) << "Net " << _net->get_net_name() << " has no pins";
    auto pins = get_pins();
    auto x = 0;
    auto y = 0;
    for (auto pin : pins) {
      x += pin->get_instance()->get_location().x();
      y += pin->get_instance()->get_location().y();
    }
    return Point(x / static_cast<int>(pins.size()), y / static_cast<int>(pins.size()));
  }
  NetType netType() const
  {
    if (CTSAPIInst.isTop(_net->get_net_name())) {
      return NetType::kTop;
    }
    int buf_num = 0;
    int sink_num = 0;
    for (auto* load : _net->get_load_insts()) {
      if (load->get_type() == CtsInstanceType::kBuffer) {
        ++buf_num;
      } else {
        ++sink_num;
      }
    }
    if (sink_num > buf_num) {
      return NetType::kLeaf;
    }
    return NetType::kTrunk;
  }
  double centerAvgDist() const
  {
    auto* config = CTSAPIInst.get_config();
    auto center = getCenterPoint();
    double dist = 0;
    for (auto* inst : _net->get_instances()) {
      dist += 1.0 * pgl::manhattan_distance(center, inst->get_location()) / config->get_micron_dbu();
    }
    return dist / static_cast<double>(_net->get_instances().size());
  }
  double getHPWL() const
  {
    auto* config = CTSAPIInst.get_config();
    int l_x = std::numeric_limits<int>::max();
    int l_y = std::numeric_limits<int>::max();
    int r_x = 0;
    int r_y = 0;
    for (auto* inst : _net->get_instances()) {
      auto loc = inst->get_location();
      l_x = std::min(l_x, loc.x());
      l_y = std::min(l_y, loc.y());
      r_x = std::max(r_x, loc.x());
      r_y = std::max(r_y, loc.y());
    }
    return 1.0 * (r_x - l_x + r_y - l_y) / config->get_micron_dbu();
  }
  double getWireLength() const
  {
    auto* config = CTSAPIInst.get_config();
    double net_len = 0.0;
    for (auto& signal_wire : _net->get_signal_wires()) {
      auto length = 1.0 * signal_wire.getWireLength() / config->get_micron_dbu();
      net_len += length;
    }
    return net_len;
  }

 private:
  CtsNet* _net;
  std::unordered_map<std::string, CtsInstance*> _name_to_inst;
};
}  // namespace icts