#pragma once

#include "LayerCoord.hpp"
#include "Orientation.hpp"
#include "RTU.hpp"
#include "RTUtil.hpp"

namespace irt {

#if 1  // astar
enum class TANodeState
{
  kNone = 0,
  kOpen = 1,
  kClose = 2
};
#endif

class TANode : public LayerCoord
{
 public:
  TANode() = default;
  ~TANode() = default;

  // getter
  std::map<Orientation, TANode*>& get_neighbor_ptr_map() { return _neighbor_ptr_map; }
  std::map<Orientation, std::set<irt_int>>& get_obs_task_map() { return _obs_task_map; }
  std::map<Orientation, std::set<irt_int>>& get_cost_task_map() { return _cost_task_map; }
  std::queue<irt_int>& get_task_queue() { return _task_queue; }
  // setter
  void set_neighbor_ptr_map(const std::map<Orientation, TANode*>& neighbor_ptr_map) { _neighbor_ptr_map = neighbor_ptr_map; }
  void set_obs_task_map(const std::map<Orientation, std::set<irt_int>>& obs_task_map) { _obs_task_map = obs_task_map; }
  void set_cost_task_map(const std::map<Orientation, std::set<irt_int>>& cost_task_map) { _cost_task_map = cost_task_map; }
  void set_task_queue(const std::queue<irt_int>& task_queue) { _task_queue = task_queue; }
  // function
  TANode* getNeighborNode(Orientation orientation)
  {
    TANode* neighbor_node = nullptr;
    if (RTUtil::exist(_neighbor_ptr_map, orientation)) {
      neighbor_node = _neighbor_ptr_map[orientation];
    }
    return neighbor_node;
  }
  bool isOBS(irt_int task_idx, Orientation orientation)
  {
    bool is_obs = false;
    if (RTUtil::exist(_obs_task_map, orientation)) {
      is_obs = RTUtil::exist(_obs_task_map[orientation], task_idx) ? false : true;
    }
    if (!is_obs) {
      is_obs = !_task_queue.empty();
    }
    return is_obs;
  }
  double getCost(irt_int task_idx, Orientation orientation)
  {
    double cost = 0;
    if (RTUtil::exist(_cost_task_map, orientation)) {
      std::set<irt_int>& task_idx_set = _cost_task_map[orientation];
      cost += RTUtil::exist(task_idx_set, task_idx) ? 0 : task_idx_set.size();
    }
    cost *= std::pow(_task_queue.size(), 2);
    return cost;
  }
  void addDemand(irt_int task_idx) { _task_queue.push(task_idx); }
#if 1  // astar
  TANodeState& get_state() { return _state; }
  TANode* get_parent_node() const { return _parent_node; }
  double get_known_cost() const { return _known_cost; }
  double get_estimated_cost() const { return _estimated_cost; }
  void set_state(TANodeState state) { _state = state; }
  void set_parent_node(TANode* parent_node) { _parent_node = parent_node; }
  void set_known_cost(const double known_cost) { _known_cost = known_cost; }
  void set_estimated_cost(const double estimated_cost) { _estimated_cost = estimated_cost; }
  bool isNone() { return _state == TANodeState::kNone; }
  bool isOpen() { return _state == TANodeState::kOpen; }
  bool isClose() { return _state == TANodeState::kClose; }
  double getTotalCost() { return (_known_cost + _estimated_cost); }
#endif

 private:
  std::map<Orientation, TANode*> _neighbor_ptr_map;
  std::map<Orientation, std::set<irt_int>> _obs_task_map;
  std::map<Orientation, std::set<irt_int>> _cost_task_map;
  std::queue<irt_int> _task_queue;
#if 1  // astar
  TANodeState _state = TANodeState::kNone;
  TANode* _parent_node = nullptr;
  double _known_cost = 0.0;  // include curr
  double _estimated_cost = 0.0;
#endif
};

#if 1  // astar
struct CmpTANodeCost
{
  bool operator()(TANode* a, TANode* b)
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
