#pragma once

#include "LayerCoord.hpp"

namespace irt {

#if 1  // astar
enum class GRNodeState
{
  kNone = 0,
  kOpen = 1,
  kClose = 2
};
#endif

class GRNode : public LayerCoord
{
 public:
  GRNode() = default;
  ~GRNode() = default;

  // getter
  PlanarRect& get_real_rect() { return _real_rect; }
  std::map<Orientation, GRNode*>& get_neighbor_ptr_map() { return _neighbor_ptr_map; }
  std::map<irt_int, std::vector<PlanarRect>>& get_net_blockage_map() { return _net_blockage_map; }
  std::map<irt_int, std::vector<PlanarRect>>& get_net_region_map() { return _net_region_map; }
  irt_int get_single_wire_area() const { return _single_wire_area; }
  irt_int get_single_via_area() const { return _single_via_area; }
  irt_int get_wire_area_supply() const { return _wire_area_supply; }
  irt_int get_via_area_supply() const { return _via_area_supply; }
  irt_int get_wire_area_demand() const { return _wire_area_demand; }
  irt_int get_via_area_demand() const { return _via_area_demand; }
  std::map<irt_int, std::set<Orientation>>& get_net_access_map() { return _net_access_map; }
  std::queue<irt_int>& get_net_queue() { return _net_queue; }
  // setter
  void set_real_rect(const PlanarRect& real_rect) { _real_rect = real_rect; }
  void set_neighbor_ptr_map(const std::map<Orientation, GRNode*>& neighbor_ptr_map) { _neighbor_ptr_map = neighbor_ptr_map; }
  void set_net_blockage_map(const std::map<irt_int, std::vector<PlanarRect>>& net_blockage_map) { _net_blockage_map = net_blockage_map; }
  void set_net_region_map(const std::map<irt_int, std::vector<PlanarRect>>& net_region_map) { _net_region_map = net_region_map; }
  void set_single_wire_area(const irt_int single_wire_area) { _single_wire_area = single_wire_area; }
  void set_single_via_area(const irt_int single_via_area) { _single_via_area = single_via_area; }
  void set_wire_area_supply(const irt_int wire_area_supply) { _wire_area_supply = wire_area_supply; }
  void set_via_area_supply(const irt_int via_area_supply) { _via_area_supply = via_area_supply; }
  void set_wire_area_demand(const irt_int wire_area_demand) { _wire_area_demand = wire_area_demand; }
  void set_via_area_demand(const irt_int via_area_demand) { _via_area_demand = via_area_demand; }
  void set_net_access_map(const std::map<irt_int, std::set<Orientation>>& net_access_map) { _net_access_map = net_access_map; }
  void set_net_queue(const std::queue<irt_int>& net_queue) { _net_queue = net_queue; }
  // function
  GRNode* getNeighborNode(Orientation orientation)
  {
    GRNode* neighbor_node = nullptr;
    if (RTUtil::exist(_neighbor_ptr_map, orientation)) {
      neighbor_node = _neighbor_ptr_map[orientation];
    }
    return neighbor_node;
  }
  bool isOBS(irt_int net_idx, Orientation orientation)
  {
    bool is_obs = true;
    if (RTUtil::exist(_net_access_map, net_idx) && RTUtil::exist(_net_access_map[net_idx], orientation)) {
      // 存在绿通
      is_obs = false;
    } else if (orientation == Orientation::kUp || orientation == Orientation::kDown) {
      // wire剩余可以给via
      is_obs = ((_wire_area_supply - _wire_area_demand + _via_area_supply - _via_area_demand) < _single_via_area);
    } else {
      // via剩余不可转wire
      is_obs = ((_wire_area_supply - _wire_area_demand + std::min(_via_area_supply - _via_area_demand, 0)) < _single_wire_area);
    }
    return is_obs;
  }
  double getCost(irt_int net_idx, Orientation orientation)
  {
    double cost = 0;
    if (RTUtil::exist(_net_access_map, net_idx) && RTUtil::exist(_net_access_map[net_idx], orientation)) {
      // 存在绿通
      cost += 0;
    } else if (orientation == Orientation::kUp || orientation == Orientation::kDown) {
      cost += RTUtil::sigmoid(_via_area_demand, _wire_area_supply - _wire_area_demand + _via_area_supply);
    } else {
      cost += RTUtil::sigmoid(_wire_area_demand, _wire_area_supply + std::min(_via_area_supply - _via_area_demand, 0));
    }
    if (!RTUtil::exist(_net_region_map, net_idx)) {
      cost += static_cast<double>(_net_region_map.size());
    }
    return cost;
  }
  void addWireDemand(irt_int net_idx)
  {
    if (RTUtil::exist(_net_access_map, net_idx)) {
      return;
    }
    _wire_area_demand += _single_wire_area;
    _net_queue.push(net_idx);
  }
  void addViaDemand(irt_int net_idx)
  {
    if (RTUtil::exist(_net_access_map, net_idx)) {
      return;
    }
    _via_area_demand += _single_via_area;
    _net_queue.push(net_idx);
  }
#if 1  // astar
  GRNodeState& get_state() { return _state; }
  GRNode* get_parent_node() const { return _parent_node; }
  double get_known_cost() const { return _known_cost; }
  double get_estimated_cost() const { return _estimated_cost; }
  void set_state(GRNodeState state) { _state = state; }
  void set_parent_node(GRNode* parent_node) { _parent_node = parent_node; }
  void set_known_cost(const double known_cost) { _known_cost = known_cost; }
  void set_estimated_cost(const double estimated_cost) { _estimated_cost = estimated_cost; }
  bool isNone() { return _state == GRNodeState::kNone; }
  bool isOpen() { return _state == GRNodeState::kOpen; }
  bool isClose() { return _state == GRNodeState::kClose; }
  double getTotalCost() { return (_known_cost + _estimated_cost); }
#endif

 private:
  PlanarRect _real_rect;
  std::map<Orientation, GRNode*> _neighbor_ptr_map;
  std::map<irt_int, std::vector<PlanarRect>> _net_blockage_map;
  std::map<irt_int, std::vector<PlanarRect>> _net_region_map;
  irt_int _single_wire_area = 0;
  irt_int _single_via_area = 0;
  irt_int _wire_area_supply = 0;
  irt_int _via_area_supply = 0;
  irt_int _wire_area_demand = 0;
  irt_int _via_area_demand = 0;
  // 使用绿通时，不消耗资源
  std::map<irt_int, std::set<Orientation>> _net_access_map;
  std::queue<irt_int> _net_queue;
#if 1  // astar
  GRNodeState _state = GRNodeState::kNone;
  GRNode* _parent_node = nullptr;
  double _known_cost = 0.0;  // include curr
  double _estimated_cost = 0.0;
#endif
};

#if 1  // astar
struct CmpGRNodeCost
{
  bool operator()(GRNode* a, GRNode* b)
  {
    if (RTUtil::equalDoubleByError(a->getTotalCost(), b->getTotalCost(), DBL_ERROR)) {
      if (RTUtil::equalDoubleByError(a->get_estimated_cost(), b->get_estimated_cost(), DBL_ERROR)) {
        return a->get_neighbor_ptr_map().size() < b->get_neighbor_ptr_map().size();
      } else {
        return a->get_estimated_cost() > b->get_estimated_cost();
      }
    } else {
      return a->getTotalCost() > b->getTotalCost();
    }
  }
};
#endif

}  // namespace irt
