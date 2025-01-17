/**
 * @file StaIncremental.hh
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The incremental static timing analysis DAG graph.
 * @version 0.1
 * @date 2022-01-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <queue>

#include "StaFunc.hh"
#include "StaVertex.hh"

namespace ista {

/**
 * @brief The top class for incremental static timing analysis.
 *
 */
class StaIncremental {
 public:
  StaIncremental();
  ~StaIncremental() = default;

  const std::function<bool(StaVertex*, StaVertex*)> cmp =
      [](StaVertex* left, StaVertex* right) -> bool {
    unsigned left_level = left->get_level();
    unsigned right_level = right->get_level();
    return left_level < right_level;
  };

  unsigned propagateSlew(StaVertex* the_vertex);
  unsigned propagateDelay(StaVertex* the_vertex);
  unsigned propagateAT(StaVertex* the_vertex);
  unsigned propagateRT(StaVertex* the_vertex);

  void insertFwdQueue(StaVertex* the_vertex);
  void insertBwdQueue(StaVertex* the_vertex);

  unsigned applyFwdQueue();
  unsigned applyBwdQueue();

 private:
  std::priority_queue<StaVertex*, std::vector<StaVertex*>, decltype(cmp)>
      _fwd_queue;
  std::priority_queue<StaVertex*, std::vector<StaVertex*>, decltype(cmp)>
      _bwd_queue;
};

/**
 * @brief reset the propagation flag or data, anything etc.
 *
 */
class StaResetPropagation : public StaFunc {
 public:
  StaResetPropagation() = default;
  ~StaResetPropagation() override = default;

  void set_is_fwd() { _is_fwd = true; }
  void set_is_bwd() { _is_fwd = false; }

  void set_incr_func(StaIncremental* incr_func) { _incr_func = incr_func; }
  void set_max_min_level(unsigned max_min_level) {
    _max_min_level = max_min_level;
  }
  bool beyondLevel(unsigned level) {
    return _is_fwd ? (_max_min_level && (level > _max_min_level))
                   : (_max_min_level && (level < _max_min_level));
  }

  unsigned operator()(StaVertex* the_vertex) override;
  unsigned operator()(StaArc* the_arc) override;

 private:
  bool _is_fwd = true;                     //!< Whether fwd propagation reset.
  std::optional<unsigned> _max_min_level;  //!< The max level of the graph.
  StaIncremental* _incr_func = nullptr;    //!< The incremental function.
};

}  // namespace ista
