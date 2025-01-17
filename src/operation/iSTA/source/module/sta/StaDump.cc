/**
 * @file StaDump.cc
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief Dump the Sta data
 * @version 0.1
 * @date 2021-04-22
 */

#include "StaDump.hh"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <regex>
#include <string>

#include "ThreadPool/ThreadPool.h"

namespace ista {

/**
 * @brief Dump the vertex data.
 *
 * @param the_vertex
 * @return unsigned
 */
unsigned StaDumpYaml::operator()(StaVertex* the_vertex) {
  YAML::Node& node = _node;

  StaVertex* vertex = the_vertex;

  static int node_id = 0;
  std::string node_name = Str::printf("node_%d", node_id++);
  YAML::Node vertex_node;
  node[node_name] = vertex_node;

  vertex_node["name"] = vertex->getNameWithCellName();
  YAML::Node node1;
  vertex_node["attribute"] = node1;
  node1["is_clock"] = vertex->is_clock();
  node1["is_port"] = vertex->is_port();
  node1["is_start"] = vertex->is_start();
  node1["is_end"] = vertex->is_end();
  node1["is_const"] = vertex->is_const();
  node1["color"] = vertex->isWhite() ? 0 : vertex->isGray() ? 1 : 2;
  node1["level"] = vertex->get_level();
  node1["is_slew_propagated"] = vertex->is_slew_prop();
  node1["is_delay_propagated"] = vertex->is_delay_prop();
  node1["is_fwd"] = vertex->is_fwd();
  node1["is_bwd"] = vertex->is_bwd();

  YAML::Node node2;
  vertex_node["src_arcs"] = node2;
  FOREACH_SRC_ARC(vertex, src_arc) {
    node2.push_back(src_arc->get_snk()->getName());
  }
  YAML::Node node3;
  vertex_node["snk_arcs"] = node3;
  FOREACH_SNK_ARC(vertex, snk_arc) {
    node3.push_back(snk_arc->get_src()->getName());
  }

  YAML::Node node4;
  vertex_node["slew_data"] = node4;
  StaData* data;
  FOREACH_SLEW_DATA(vertex, data) {
    auto* slew_data = dynamic_cast<StaSlewData*>(data);
    double slew_value = FS_TO_NS(slew_data->get_slew());
    auto delay_type = slew_data->get_delay_type();
    auto trans_type = slew_data->get_trans_type();

    auto* src_slew_data = slew_data->get_bwd();
    auto* bwd_vertex =
        src_slew_data ? src_slew_data->get_own_vertex() : nullptr;

    const char* slew_str = Str::printf(
        "%p %.3f %s %s src_vertex %s src_slew %p", slew_data, slew_value,
        delay_type == AnalysisMode::kMax ? "max" : "min",
        trans_type == TransType::kRise ? "rise" : "fall",
        bwd_vertex ? bwd_vertex->getName().c_str() : "Nil",
        src_slew_data ? src_slew_data : 0);

    node4.push_back(slew_str);
  }

  YAML::Node node5;
  vertex_node["clock_data"] = node5;
  FOREACH_CLOCK_DATA(vertex, data) {
    auto* clock_data = dynamic_cast<StaClockData*>(data);
    double arrive_time_value = FS_TO_NS(clock_data->get_arrive_time());
    auto delay_type = clock_data->get_delay_type();
    auto trans_type = clock_data->get_trans_type();

    auto* src_clock_data = clock_data->get_bwd();
    auto* bwd_vertex =
        src_clock_data ? src_clock_data->get_own_vertex() : nullptr;

    const char* data_str = Str::printf(
        "%p %.3f %s %s src_vertex %s src_data %p own clock %s", clock_data,
        arrive_time_value, delay_type == AnalysisMode::kMax ? "max" : "min",
        trans_type == TransType::kRise ? "rise" : "fall",
        bwd_vertex ? bwd_vertex->getName().c_str() : "Nil",
        src_clock_data ? src_clock_data : 0,
        clock_data->get_prop_clock()->get_clock_name());

    node5.push_back(data_str);
  }

  YAML::Node node6;
  vertex_node["path_data"] = node6;
  FOREACH_DELAY_DATA(vertex, data) {
    auto* path_delay = dynamic_cast<StaPathDelayData*>(data);
    double arrive_time_value = FS_TO_NS(path_delay->get_arrive_time());
    auto req_time_value = FS_TO_NS(path_delay->get_req_time().value_or(0));
    auto delay_type = path_delay->get_delay_type();
    auto trans_type = path_delay->get_trans_type();

    auto* src_data = path_delay->get_bwd();
    auto* src_vertex = src_data ? src_data->get_own_vertex() : nullptr;

    const char* data_str =
        Str::printf("%p AT: %.3f RT: %.3f %s %s src_vertex %s src_data %p",
                    path_delay, arrive_time_value, req_time_value,
                    delay_type == AnalysisMode::kMax ? "max" : "min",
                    trans_type == TransType::kRise ? "rise" : "fall",
                    src_vertex ? src_vertex->getName().c_str() : "Nil",
                    src_data ? src_data : 0);

    node6.push_back(data_str);
  }

  return 1;
}

/**
 * @brief Dump the arc data.
 *
 * @param the_arc
 * @return unsigned
 */
unsigned StaDumpYaml::operator()(StaArc* the_arc) {
  StaArc* arc = the_arc;
  YAML::Node& node = _node;

  static int node_id = 0;
  std::string node_name = Str::printf("arc_%d", node_id++);
  YAML::Node arc_node;
  node[node_name] = arc_node;

  arc_node["src"] = arc->get_src()->getName();
  arc_node["snk"] = arc->get_snk()->getName();
  arc_node["arc_type"] =
      arc->isDelayArc()
          ? (arc->isInstArc() ? "cell Delay" : "net Delay")
          : arc->isSetupArc() ? "Setup" : arc->isHoldArc() ? "Hold" : "Other";

  YAML::Node delay_node;
  arc_node["arc_delay_data"] = delay_node;

  StaData* data;
  FOREACH_ARC_DELAY_DATA(arc, data) {
    auto* arc_delay_data = dynamic_cast<StaArcDelayData*>(data);
    auto delay_type = arc_delay_data->get_delay_type();
    auto trans_type = arc_delay_data->get_trans_type();
    auto arc_delay_value = arc_delay_data->get_arc_delay();

    const char* arc_delay_str =
        Str::printf("%d %s %s", arc_delay_value,
                    delay_type == AnalysisMode::kMax ? "max" : "min",
                    trans_type == TransType::kRise ? "rise" : "fall");

    delay_node.push_back(arc_delay_str);
  }

  return 1;
}

/**
 * @brief Print in yaml for text
 *
 * @param the_graph
 * @return unsigned 1 if success, 0 else fail.
 */
unsigned StaDumpYaml::operator()(StaGraph* the_graph) {
  LOG_INFO << "dump graph yaml start";

  LOG_INFO << "dump graph node total node " << the_graph->numVertex();
  LOG_INFO << "dump graph arc total arc " << the_graph->numArc();

  StaVertex* the_vertex;
  FOREACH_VERTEX(the_graph, the_vertex) { the_vertex->exec(*this); }

  StaArc* the_arc;
  FOREACH_ARC(the_graph, the_arc) { the_arc->exec(*this); }

  printText("graph.yaml");

  LOG_INFO << "dump graph yaml end";
  return 1;
}

/**
 * @brief Print the yaml to text.
 *
 * @param file_name
 */
void StaDumpYaml::printText(const char* file_name) {
  std::ofstream file(file_name, std::ios::trunc);
  file << _node << std::endl;
  file.close();
}

/**
 * @brief Print Graph in GraphViz dot format.
 *
 * @param the_graph
 * @return unsigned 1 if success, 0 else fail.
 */
unsigned StaDumpGraphViz::operator()(StaGraph* the_graph) {
  LOG_INFO << "dump graph dotviz start";

  std::ofstream dot_file;
  dot_file.open("digraph.dot");

  dot_file << "digraph g {"
           << "\n";

  StaArc* arc;
  FOREACH_ARC(the_graph, arc) {
    auto src_name = arc->get_src()->getName();
    auto snk_name = arc->get_snk()->getName();

    auto new_src_name = Str::replace(src_name, "[/:\\[\\]]", "_");
    auto new_snk_name = Str::replace(snk_name, "[/:\\[\\]]", "_");

    auto max_rise_delay =
        arc->get_arc_delay(AnalysisMode::kMax, TransType::kRise);
    auto max_fall_delay =
        arc->get_arc_delay(AnalysisMode::kMax, TransType::kFall);
    auto min_rise_delay =
        arc->get_arc_delay(AnalysisMode::kMin, TransType::kRise);
    auto min_fall_delay =
        arc->get_arc_delay(AnalysisMode::kMin, TransType::kRise);

    std::string label =
        Str::printf("[label = \" ps ar %.3f af %.3f ir %.3f if %.3f\"]",
                    FS_TO_PS(max_rise_delay), FS_TO_PS(max_fall_delay),
                    FS_TO_PS(min_rise_delay), FS_TO_PS(min_fall_delay));

    dot_file << new_src_name << " -> " << new_snk_name << " " << label << "\n";
  }

  dot_file << "}"
           << "\n";

  dot_file.close();

  LOG_INFO << "dump graph dotviz end";

  return 1;
}

}  // namespace ista