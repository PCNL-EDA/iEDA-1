#pragma once

#include "GRNetPriority.hpp"
#include "GRPin.hpp"
#include "LayerCoord.hpp"
#include "MTree.hpp"
#include "Net.hpp"

namespace irt {

class GRNet
{
 public:
  GRNet() = default;
  ~GRNet() = default;

  // getter
  Net* get_origin_net() { return _origin_net; }
  irt_int get_net_idx() const { return _net_idx; }
  ConnectType get_connect_type() const { return _connect_type; }
  std::vector<GRPin>& get_gr_pin_list() { return _gr_pin_list; }
  GRPin& get_gr_driving_pin() { return _gr_driving_pin; }
  BoundingBox& get_bounding_box() { return _bounding_box; }
  GridMap<double>& get_ra_cost_map() { return _ra_cost_map; }
  GRNetPriority& get_gr_net_priority() { return _gr_net_priority; }
  std::vector<Segment<LayerCoord>>& get_routing_segment_list() { return _routing_segment_list; }
  MTree<RTNode>& get_gr_result_tree() { return _gr_result_tree; }
  // setter
  void set_origin_net(Net* origin_net) { _origin_net = origin_net; }
  void set_net_idx(const irt_int net_idx) { _net_idx = net_idx; }
  void set_connect_type(const ConnectType& connect_type) { _connect_type = connect_type; };
  void set_gr_pin_list(std::vector<GRPin>& gr_pin_list) { _gr_pin_list = gr_pin_list; }
  void set_gr_driving_pin(const GRPin& gr_driving_pin) { _gr_driving_pin = gr_driving_pin; }
  void set_bounding_box(const BoundingBox& bounding_box) { _bounding_box = bounding_box; }
  void set_ra_cost_map(const GridMap<double>& ra_cost_map) { _ra_cost_map = ra_cost_map; }
  void set_gr_net_priority(const GRNetPriority& gr_net_priority) { _gr_net_priority = gr_net_priority; }
  void set_routing_segment_list(const std::vector<Segment<LayerCoord>>& routing_segment_list)
  {
    _routing_segment_list = routing_segment_list;
  }
  void set_gr_result_tree(const MTree<RTNode>& gr_result_tree) { _gr_result_tree = gr_result_tree; }

 private:
  Net* _origin_net = nullptr;
  irt_int _net_idx = -1;
  ConnectType _connect_type = ConnectType::kNone;
  std::vector<GRPin> _gr_pin_list;
  GRPin _gr_driving_pin;
  BoundingBox _bounding_box;
  GridMap<double> _ra_cost_map;
  GRNetPriority _gr_net_priority;
  std::vector<Segment<LayerCoord>> _routing_segment_list;
  MTree<RTNode> _gr_result_tree;
};

}  // namespace irt
