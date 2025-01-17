/**
 * @file StaBuildTag.hh
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief Build prop tag for user specify -from -through -to.
 * @version 0.1
 * @date 2022-04-21
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once

#include "sta/StaFunc.hh"
#include "sta/StaVertex.hh"
namespace ista {

/**
 * @brief The -from -through -to constructed subgraph.
 *
 */
class StaSubGraph {
 public:
  void addPropArc(StaArc* prop_arc) { _prop_arcs.insert(prop_arc); }
  auto& get_prop_arcs() { return _prop_arcs; }
  void set_prop_arcs(std::set<StaArc*>&& prop_arcs) {
    _prop_arcs = std::move(prop_arcs);
  }

  StaSubGraph intersectSubGraph(StaSubGraph& other);
  void markSubGraphPropTag(StaPropagationTag::TagType tag_type);

 private:
  std::set<StaArc*> _prop_arcs;
};

/**
 * @brief Build the tag of propagation.
 *
 */
class StaBuildPropTag : StaFunc {
 public:
  explicit StaBuildPropTag(StaPropagationTag::TagType tag_type)
      : _tag_type(tag_type) {}
  ~StaBuildPropTag() override = default;

  enum class PropStat {
    kPropPath = 0,
    kCollectPath = 1,
  };
  enum class PointType { kStart, kEnd, kThrough };

  unsigned operator()(StaVertex* the_vertex) override;
  unsigned operator()(StaGraph* the_graph) override;

 protected:
  void setPropPath() { _prop_stat = PropStat::kPropPath; }
  bool isPropPath() { return _prop_stat == PropStat::kPropPath; }
  void setCollectPath() { _prop_stat = PropStat::kCollectPath; }
  bool isCollectPath() { return _prop_stat == PropStat::kCollectPath; }
  auto get_tag_type() { return _tag_type; }
  void set_tag_type(StaPropagationTag::TagType tag_type) {
    _tag_type = tag_type;
  }

  auto get_prop_stat() { return _prop_stat; }

  void reservedSubGraph() { _sub_graphs.emplace_back(); }
  auto& getCurrSubGraph() { return _sub_graphs.back(); }
  auto& get_sub_graphs() { return _sub_graphs; }

  void setFromThroughToTag(StaGraph* the_graph,
                           std::vector<std::string>& prop_vec,
                           PointType tag_type);
  void setFromToTag(StaGraph* the_graph, std::vector<std::string>& from_vec,
                    std::vector<std::string>& to_vec);
  void resetVertexTagData(StaGraph* the_graph);
  StaSubGraph buildFinalSubGraph();
  unsigned buildTagGraph(
      StaGraph* the_graph, std::vector<std::string> prop_froms,
      std::vector<std::string> prop_tos,
      std::vector<std::vector<std::string>> prop_throughs_list);

 private:
  PropStat _prop_stat;
  std::vector<StaSubGraph> _sub_graphs;

  StaPropagationTag::TagType _tag_type = StaPropagationTag::TagType::kProp;
};

}  // namespace ista