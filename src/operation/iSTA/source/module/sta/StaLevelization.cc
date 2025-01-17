/**
 * @file StaLevelization.cc
 * @author shy long (longshy@pcl.ac.cn)
 * @brief
 * @version 0.1
 * @date 2021-09-16
 */

#include "StaLevelization.hh"

namespace ista {

/**
 * @brief Levelization from the vertex.
 *
 * @param the_vertex
 * @return unsigned  1 if success, 0 else fail.
 */
unsigned StaLevelization::operator()(StaVertex* the_vertex) {
  if (the_vertex->is_start()) {
    the_vertex->set_level(1);
    return 1;
  }

  if (the_vertex->isSetLevel()) {
    return 1;
  }

  FOREACH_SNK_ARC(the_vertex, snk_arc) {
    if (snk_arc->isDelayArc()) {
      snk_arc->exec(*this);
    }
  }

  return 1;
}

/**
 * @brief Levelization from the arc.
 *
 * @param the_arc
 * @return unsigned  1 if success, 0 else fail.
 */
unsigned StaLevelization::operator()(StaArc* the_arc) {
  auto* src_vertex = the_arc->get_src();
  auto* snk_vertex = the_arc->get_snk();
  src_vertex->exec(*this);

  unsigned src_level = src_vertex->get_level();
  snk_vertex->set_level(src_level + 1);

  return 1;
}

/**
 * @brief Levelization from the graph port vertex.
 *
 * @param the_graph
 * @return unsigned  1 if success, 0 else fail.
 */
unsigned StaLevelization::operator()(StaGraph* the_graph) {
  StaVertex* end_vertex;

  FOREACH_END_VERTEX(the_graph, end_vertex) { end_vertex->exec(*this); }

  return 1;
}

}  // namespace ista