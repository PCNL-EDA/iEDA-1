/**
 * @file PwrBuildSeqGraph.cc
 * @author shaozheqing (707005020@qq.com)
 * @brief Build the sequential graph.
 * @version 0.1
 * @date 2023-03-03
 */

#include "PwrBuildSeqGraph.hh"

#include <condition_variable>
#include <mutex>
#include <ranges>

#include "ThreadPool/ThreadPool.h"
#include "ops/dump/PwrDumpSeqGraph.hh"

namespace ipower {
using ieda::Stats;

/**
 * @brief Build seq arc from the power vertex, deep to the end use the dfs.
 *
 * @param the_vertex
 * @return unsigned
 */
unsigned PwrBuildSeqGraph::operator()(PwrVertex* the_vertex) {
  std::lock_guard lk(the_vertex->get_mutex());
  if (the_vertex->is_seq_visited()) {
    return 1;
  }

  /*Lambda function to create seq graph arc.*/
  auto create_seq_arc = [this](auto* seq_vertex, auto* fanout_seq_vertex) {
    PwrSeqArc* seq_arc = new PwrSeqArc(seq_vertex, fanout_seq_vertex);
    seq_vertex->addSrcArc(seq_arc);
    fanout_seq_vertex->addSnkArc(seq_arc);
    _seq_graph.addPwrSeqArc(seq_arc);
  };

  auto* the_sta_vertex = the_vertex->get_sta_vertex();
  if (the_sta_vertex->is_end()) {
    /*When the snk vertex is a port, add to fanout vertexes..*/
    if (the_sta_vertex->is_port()) {
      PwrVertex* the_pwr_vertex =
          get_the_pwr_graph()->staToPwrVertex(the_sta_vertex);
      PwrSeqVertex* seq_vertex = _seq_graph.getPortSeqVertex(the_pwr_vertex);
      the_vertex->addFanoutSeqVertex(seq_vertex);
      the_vertex->set_is_seq_visited();
      return 1;
    }
    // TODO clock gate need consider lower power.
    if (the_sta_vertex->is_clock_gate_end()) {
      the_vertex->set_is_seq_visited();
      return 1;
    }
    /*Find a seq datain vertex, add to fanout vertexes.*/
    Instance* seq_instance = the_vertex->getOwnInstance();
    PwrSeqVertex* seq_vertex = _seq_graph.getSeqVertex(seq_instance);
    the_vertex->addFanoutSeqVertex(seq_vertex);
    the_vertex->set_is_seq_visited();
    return 1;
  }

  /*find snk vertexes of the vertex.*/
  auto& src_arcs = the_sta_vertex->get_src_arcs();
  for (auto& src_arc : src_arcs) {
    if (!src_arc->isDelayArc()) {
      continue;
    }

    // if the snk vertex is clock pin, dont consider the clock path.
    StaVertex* snk_vertex = src_arc->get_snk();
    if (snk_vertex->is_clock()) {
      continue;
    }

    PwrVertex* snk_pwr_vertex = get_the_pwr_graph()->staToPwrVertex(snk_vertex);
    if (snk_pwr_vertex->exec(*this)) {
      auto& fanout_seq_vertexes = snk_pwr_vertex->get_fanout_seq_vertexes();
      // The vertex is comb, add the fauout seq vertexes to the set.
      the_vertex->addFanoutSeqVertex(fanout_seq_vertexes);
    }
  }

  auto& fanout_seq_vertexes = the_vertex->get_fanout_seq_vertexes();

  if (the_vertex->get_own_seq_vertex()) {
    // The vertex is seq, create all seq arcs by the set of fanout seq
    // vertexes.
    for (auto* fanout_seq_vertex : fanout_seq_vertexes) {
      create_seq_arc(the_vertex->get_own_seq_vertex(), fanout_seq_vertex);
    }
  }

  the_vertex->set_is_seq_visited();
  return 1;
}

/**
 * @brief Build seq vertexes for seq instance.
 *
 * @param the_graph
 * @return unsigned
 */
unsigned PwrBuildSeqGraph::buildSeqVertexes(PwrGraph* the_graph) {
  /*Lambda function of find clock to dataout vertexes.*/
  auto find_clk_to_out_vertex = [the_graph](StaVertex* clk_vertex) {
    Set<PwrVertex*> data_out_pwr_vertexes;
    auto& clk_src_arcs = clk_vertex->get_src_arcs();
    for (auto* src_arc : clk_src_arcs | std::views::filter([](auto* src_arc) {
                           return src_arc->isDelayArc();
                         })) {
      StaVertex* data_out_sta_vertex = src_arc->get_snk();
      PwrVertex* data_out_pwr_vertex =
          the_graph->staToPwrVertex(data_out_sta_vertex);
      data_out_pwr_vertexes.insert(data_out_pwr_vertex);
    }
    return data_out_pwr_vertexes;
  };

  /*Lamdba function of find clock to datain vertexes.*/
  auto find_clk_to_in_vertex = [the_graph](StaVertex* clk_vertex) {
    Set<PwrVertex*> data_in_pwr_vertexes;
    std::vector<StaArc*> check_arcs =
        clk_vertex->getSrcCheckArcs(AnalysisMode::kMax);
    for (auto* check_arc : check_arcs) {
      StaVertex* data_in_sta_vertex = check_arc->get_snk();
      PwrVertex* data_in_pwr_vertex =
          the_graph->staToPwrVertex(data_in_sta_vertex);
      data_in_pwr_vertexes.insert(data_in_pwr_vertex);
    }
    return data_in_pwr_vertexes;
  };

  /*Lambda function of build a sequential vertex.*/
  auto build_seq_vertex = [&find_clk_to_in_vertex, &find_clk_to_out_vertex,
                           this](StaVertex* clock_vertex) {
    // Find datain vertexes from clk.
    auto data_in_pwr_vertexes = find_clk_to_in_vertex(clock_vertex);
    // Find dataout vertexes from clk.
    auto data_out_pwr_vertexes = find_clk_to_out_vertex(clock_vertex);
    // Find the instance of these vertexes.
    Instance* seq_instance = (*data_out_pwr_vertexes.begin())->getOwnInstance();
    // New a seq vertex for this instance.
    PwrSeqVertex* seq_vertex = new PwrSeqVertex(
        std::move(data_in_pwr_vertexes), std::move(data_out_pwr_vertexes));
    // Add the seq vertex into graph.
    _seq_graph.addPwrSeqVertex(seq_vertex);
    // Add instance to vertex map into graph.
    _seq_graph.insertInstToVertex(seq_instance, seq_vertex);
  };

  // Build thread pool for build vertexes.
  ThreadPool thread_pool_build_vertexes(get_num_threads());

  /*Build sequential vertexes and add them to sequential graph.*/
  StaVertex* start_vertex;
  FOREACH_START_VERTEX(the_graph->get_sta_graph(), start_vertex) {
    /*Find the start clock vertexes.*/
    if (start_vertex->is_clock()) {
/*Select whether to use multithreading for build vertexes*/
#if 1
      thread_pool_build_vertexes.enqueue(
          [this, build_seq_vertex](auto* start_vertex) {
            build_seq_vertex(start_vertex);
          },
          start_vertex);
#else
      build_seq_vertex(start_vertex);
#endif
    }
  }
  return 1;
}

/**
 * @brief Build speical seq vertexes for port vertexes.
 *
 * @param the_graph
 * @return unsigned
 */
unsigned PwrBuildSeqGraph::buildPortVertexes(PwrGraph* the_graph) {
  /*Build speical seq vertexes for input port vertexes.*/
  auto& input_port_vertexes = the_graph->get_input_port_vertexes();
  for (auto* input_port_vertex : input_port_vertexes) {
    // New a seq vertex for this input port.
    PwrSeqVertex* seq_vertex =
        new PwrSeqVertex(SeqPortType::kInput, input_port_vertex);
    // Add the seq vertex into graph.
    _seq_graph.addPwrSeqVertex(seq_vertex);
    _seq_graph.addInputPortVerex(seq_vertex);
    _seq_graph.insertPortToVertex(input_port_vertex, seq_vertex);
  }

  /*Build speical seq vertexes for input port vertexes.*/
  auto& output_port_vertexes = the_graph->get_output_port_vertexes();
  for (auto* output_port_vertex : output_port_vertexes) {
    // New a seq vertex for this input port.
    PwrSeqVertex* seq_vertex =
        new PwrSeqVertex(SeqPortType::kOutput, output_port_vertex);
    // Add the seq vertex into graph.
    _seq_graph.addPwrSeqVertex(seq_vertex);
    _seq_graph.addOutputPortVerex(seq_vertex);
    _seq_graph.insertPortToVertex(output_port_vertex, seq_vertex);
  }
  return 1;
}

/**
 * @brief Build seq arcs.
 *
 * @return unsigned
 */
unsigned PwrBuildSeqGraph::buildSeqArcs(PwrGraph* the_graph) {
  /*Lambda function of build seq arc.*/
  auto build_seq_arc = [the_graph, this](auto* seq_vertex) {
    auto& data_out_pwr_vertexes = seq_vertex->get_seq_out_vertexes();
    for (auto* data_out_pwr_vertex : data_out_pwr_vertexes) {
      StaVertex* data_out_sta_vertex =
          the_graph->pwrToStaVertex(data_out_pwr_vertex);

      // TODO Macro to be done, do not build the connecting
      // relationships
      if (auto* own_cell = data_out_sta_vertex->getOwnCell();
          own_cell && own_cell->isMacroCell()) {
        continue;
      }
      data_out_pwr_vertex->exec(*this);
    }
  };

  // Build thread pool for build arcs.
  ThreadPool thread_pool_build_arcs(get_num_threads());

  /*Build the connecting relationships between seq vertexes.*/
  auto& seq_vertexes = _seq_graph.get_vertexes();
  for (auto& seq_vertex :
       seq_vertexes | std::views::filter([](auto& seq_vertex) {
         return !seq_vertex->isOutputPort();
       })) {
    /*Select whether to use multithreading for build arcs*/
#if 1
    thread_pool_build_arcs.enqueue(build_seq_arc, seq_vertex.get());
#else
    build_seq_arc(seq_vertex.get());
#endif
  }
  return 1;
}

/**
 * @brief Build the sequential graph.
 *
 * @param the_graph
 * @return unsigned
 */
unsigned PwrBuildSeqGraph::operator()(PwrGraph* the_graph) {
  Stats stats;

  set_the_pwr_graph(the_graph);

  LOG_INFO << "build seq graph start";
  unsigned is_ok;
  is_ok = buildSeqVertexes(the_graph) && buildPortVertexes(the_graph);
  is_ok = is_ok ? buildSeqArcs(the_graph) : 0;
  LOG_INFO << "build seq graph end";

  LOG_INFO << "seq inst num: " << _seq_graph.getSeqVertexNum();
  LOG_INFO << "input port num: " << _seq_graph.getInputPortNum();
  LOG_INFO << "output port num: " << _seq_graph.getOutputPortNum();

  auto [max_fanout, max_fanin] = _seq_graph.getSeqVertexMaxFanoutAndMaxFain();
  LOG_INFO << "seq vertex max fanout: " << max_fanout;
  LOG_INFO << "seq vertex max fanin: " << max_fanin;

  double memory_delta = stats.memoryDelta();
  LOG_INFO << "build seq graph memory usage " << memory_delta << "MB";
  double time_delta = stats.elapsedRunTime();
  LOG_INFO << "build seq graph time elapsed " << time_delta << "s";
  return is_ok;
}

}  // namespace ipower