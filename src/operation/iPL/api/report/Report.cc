#include <fstream>
#include <iostream>
#include <set>

#include "TimingEval.hpp"
#include "iPL_API.hh"
#include "module/checker/layout_checker/LayoutChecker.hh"
#include "module/evaluator/density/Density.hh"
#include "module/evaluator/wirelength/HPWirelength.hh"
#include "module/evaluator/wirelength/SteinerWirelength.hh"
#include "module/logger/Log.hh"
#include "report/ReportTable.hh"
#include "usage/usage.hh"

namespace ipl {

void iPL_API::reportPLInfo()
{
  LOG_INFO << "-----------------Start iPL Report Generation-----------------";
  ieda::Stats report_status;

  std::string output_dir = "./result/pl/report/";

  // report violation info
  std::string violation_file = "violation_record.txt";
  std::ofstream violation_stream;
  violation_stream.open(output_dir + violation_file);
  if (!violation_stream.good()) {
    LOG_WARNING << "Cannot open file for violation info !";
  }
  reportLayoutInfo(violation_stream);
  violation_stream.close();
  LOG_INFO << "Violation Info Write to "
           << "'" << (output_dir + violation_file) << "'";

  // report wirelength info
  std::string wirelength_file = "wirelength_record.txt";
  std::ofstream wirelength_stream;
  wirelength_stream.open(output_dir + wirelength_file);
  if (!wirelength_stream.good()) {
    LOG_WARNING << "Cannot open file for wirelength info !";
  }
  reportHPWLInfo(wirelength_stream);
  reportSTWLInfo(wirelength_stream);
  reportLongNetInfo(wirelength_stream);
  wirelength_stream.close();
  LOG_INFO << "Wirelength Info Write to "
           << "'" << (output_dir + wirelength_file) << "'";

  // report density info
  std::string density_file = "density_record.txt";
  std::ofstream density_stream;
  density_stream.open(output_dir + density_file);
  if (!density_stream.good()) {
    LOG_WARNING << "Cannot open file for density info !";
  }
  reportPeakBinDensity(density_stream);
  density_stream.close();
  LOG_INFO << "Density Info Write to "
           << "'" << (output_dir + density_file) << "'";

  // report timing info
  if (PlacerDBInst.get_placer_config()->isTimingAwareMode()) {
    std::string timing_file = "timing_record.txt";
    std::ofstream timing_stream;
    timing_stream.open(output_dir + timing_file);
    if (!timing_stream.good()) {
      LOG_WARNING << "Cannot open file for timing info !";
    }
    reportTimingInfo(timing_stream);
    timing_stream.close();
    LOG_INFO << "Timing Info Write to "
             << "'" << (output_dir + timing_file) << "'";
  }

  // report congestion
  // reportCongestionInfo();
  LOG_INFO << "Congestion Info Write to "
           << "'" << output_dir << "'";

  double time_delta = report_status.elapsedRunTime();

  LOG_INFO << "Report Generation Total Time Elapsed: " << time_delta << "s";
  LOG_INFO << "-----------------Finish Report Generation-----------------";
}

void iPL_API::reportLayoutInfo(std::ofstream& feed)
{
  LayoutChecker* checker = new LayoutChecker(&PlacerDBInst);

  LOG_INFO << "Detect Core outside Instances...";
  std::vector<Instance*> illegal_outside_inst_list = checker->obtainIllegalInstInsideCore();
  if (static_cast<int32_t>(illegal_outside_inst_list.size()) != 0) {
    LOG_ERROR << "Illegal Outside Instances Count : " << illegal_outside_inst_list.size();
    feed << "Illegal Outside Instances Count : " << illegal_outside_inst_list.size() << std::endl;
    for (auto inst : illegal_outside_inst_list) {
      feed << "Illegal Location Instance " << inst->get_name() << " Location : " << inst->get_shape().get_ll_x() << ","
           << inst->get_shape().get_ll_y() << " " << inst->get_shape().get_ur_x() << "," << inst->get_shape().get_ur_y() << std::endl;
    }
    feed << std::endl;
  }

  LOG_INFO << "Detect Instances' Alignment...";
  std::vector<Instance*> illegal_loc_inst_list = checker->obtainIllegalInstAlignRowSite();
  if (static_cast<int32_t>(illegal_loc_inst_list.size()) != 0) {
    LOG_ERROR << "Illegal Alignment Instances Count : " << illegal_loc_inst_list.size();
    feed << "Illegal Alignment Instances Count : " << illegal_loc_inst_list.size() << std::endl;
    for (auto inst : illegal_loc_inst_list) {
      feed << "Illegal Location Instance " << inst->get_name() << " Location : " << inst->get_shape().get_ll_x() << ","
           << inst->get_shape().get_ll_y() << " " << inst->get_shape().get_ur_x() << "," << inst->get_shape().get_ur_y() << std::endl;
    }
    feed << std::endl;
  }

  LOG_INFO << "Detect Power Alignment...";
  std::vector<Instance*> illegal_power_inst_list = checker->obtainIllegalInstAlignPower();
  if (static_cast<int32_t>(illegal_power_inst_list.size()) != 0) {
    LOG_ERROR << "Illegal Power Orient Instances Count : " << illegal_power_inst_list.size();
    feed << "Illegal Power Orient Instances Count : " << illegal_power_inst_list.size() << std::endl;
    for (auto inst : illegal_power_inst_list) {
      feed << "Illegal Power Orient Instance " << inst->get_name() << " Location : " << inst->get_shape().get_ll_x() << ","
           << inst->get_shape().get_ll_y() << " " << inst->get_shape().get_ur_x() << "," << inst->get_shape().get_ur_y() << std::endl;
    }
    feed << std::endl;
  }

  LOG_INFO << "Detect Overlap Between Instances...";
  if (!checker->isNoOverlapAmongInsts()) {
    reportOverlapInfo(feed);
    LOG_ERROR << "Overlap Exist";
  }

  delete checker;
}

void iPL_API::reportLayoutWhiteInfo()
{
  LayoutChecker* checker = new LayoutChecker(&PlacerDBInst);

  auto white_site_list = checker->obtainWhiteSiteList();

  if (white_site_list.size()) {
    LOG_INFO << "Detect Sites haven't been filled Count : " << white_site_list.size();
    // int32_t dbu = PlacerDBInst.get_layout()->get_database_unit();

    std::ofstream file_stream;
    file_stream.open("./result/pl/WhiteSites.txt");
    if (!file_stream.good()) {
      LOG_WARNING << "Cannot open file for white sites !";
    }

    int32_t idx = 1;
    for (auto rect : white_site_list) {
      // file_stream << idx++ << " (ll_x,ll_y ur_x,ur_y) : " << static_cast<float>(rect.get_ll_x()) / dbu  << "," <<
      // static_cast<float>(rect.get_ll_y()) / dbu << " " << static_cast<float>(rect.get_ur_x()) / dbu << "," <<
      // static_cast<float>(rect.get_ur_y()) / dbu << std::endl;
      file_stream << idx++ << " (ll_x,ll_y ur_x,ur_y) : " << rect.get_ll_x() << "," << rect.get_ll_y() << " " << rect.get_ur_x() << ","
                  << rect.get_ur_y() << std::endl;
    }
    file_stream.close();
    LOG_INFO << "White Sites has writen to file ./result/pl/WhiteSites.txt";
  }

  delete checker;
}

void iPL_API::reportPeakBinDensity(std::ofstream& feed)
{
  auto core_shape = PlacerDBInst.get_layout()->get_core_shape();
  int32_t bin_cnt_x = PlacerDBInst.get_placer_config()->get_nes_config().get_bin_cnt_x();
  int32_t bin_cnt_y = PlacerDBInst.get_placer_config()->get_nes_config().get_bin_cnt_y();
  float target_density = PlacerDBInst.get_placer_config()->get_nes_config().get_target_density();

  GridManager grid_manager(core_shape, bin_cnt_x, bin_cnt_y, target_density);

  // add inst
  for (auto* inst : PlacerDBInst.get_design()->get_instance_list()) {
    if (inst->isOutsideInstance()) {
      continue;
    }
    if (inst->get_coordi().isUnLegal()) {
      continue;
    }
    auto inst_shape = std::move(inst->get_shape());
    std::vector<Grid*> overlap_grid_list;
    grid_manager.obtainOverlapGridList(overlap_grid_list, inst_shape);
    for (auto* grid : overlap_grid_list) {
      int64_t overlap_area = grid_manager.obtainOverlapArea(grid, inst_shape);
      grid->add_area(overlap_area);
    }
  }

  Density density_eval(&grid_manager);
  feed << "Peak BinDensity: " << density_eval.obtainPeakBinDensity() << std::endl;
  feed << std::endl;
}

// NOLINTNEXTLINE
void plotPinName(std::stringstream& feed, std::string net_name, TreeNode* tree_node)
{
  feed << "TEXT" << std::endl;
  feed << "LAYER 1" << std::endl;
  feed << "TEXTTYPE 0" << std::endl;
  feed << "PRESENTATION 0,2,0" << std::endl;
  feed << "PATHTYPE 1" << std::endl;
  feed << "STRANS 0,0,0" << std::endl;
  feed << "MAG 1875" << std::endl;
  feed << "XY" << std::endl;
  int32_t pin_x = tree_node->get_point().get_x();
  int32_t pin_y = tree_node->get_point().get_y();
  feed << pin_x << " : " << pin_y << std::endl;

  auto* node = tree_node->get_node();
  if (node) {  // the real pin.
    feed << "STRING " + node->get_name() << std::endl;
  } else {  // the stainer pin.
    feed << "STRING " + net_name + " : " + "s" << std::endl;
  }
  feed << "ENDEL" << std::endl;
}

void iPL_API::plotConnectionForDebug(std::vector<std::string> net_name_list, std::string path)
{
  std::ofstream file_stream;
  file_stream.open(path);
  if (!file_stream.good()) {
    LOG_WARNING << "Cannot open file for connection info !";
  }

  auto* ipl_layout = PlacerDBInst.get_layout();
  auto* topo_manager = PlacerDBInst.get_topo_manager();

  std::vector<NetWork*> net_list;
  for (std::string net_name : net_name_list) {
    auto* network = topo_manager->findNetwork(net_name);
    if (!network) {
      LOG_WARNING << "Net : " << net_name << " Not Found!";
    } else {
      net_list.push_back(network);
    }
  }

  SteinerWirelength stwl_eval(PlacerDBInst.get_topo_manager());
  stwl_eval.updatePartOfNetWorkPointPair(net_list);

  std::stringstream feed;
  feed << "HEADER 600" << std::endl;
  feed << "BGNLIB" << std::endl;
  feed << "LIBNAME ITDP_LIB" << std::endl;
  feed << "UNITS 0.001 1e-9" << std::endl;
  feed << "BGNSTR" << std::endl;
  feed << "STRNAME core" << std::endl;
  feed << "BOUNDARY" << std::endl;
  feed << "LAYER 0" << std::endl;
  feed << "DATATYPE 0" << std::endl;
  feed << "XY" << std::endl;

  auto core_shape = ipl_layout->get_core_shape();
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << "ENDEL" << std::endl;

  // tmp set the wire width.
  int32_t wire_width = 160;

  for (auto* network : net_list) {
    auto* multi_tree = stwl_eval.obtainMultiTree(network);
    std::set<TreeNode*> visited_tree_nodes;
    std::queue<TreeNode*> tree_node_queue;
    tree_node_queue.push(multi_tree->get_root());
    while (!tree_node_queue.empty()) {
      auto* source_tree_node = tree_node_queue.front();
      if (visited_tree_nodes.find(source_tree_node) == visited_tree_nodes.end()) {
        plotPinName(feed, network->get_name(), source_tree_node);
        visited_tree_nodes.emplace(source_tree_node);
      }
      for (auto* sink_tree_node : source_tree_node->get_child_list()) {
        plotPinName(feed, network->get_name(), sink_tree_node);
        visited_tree_nodes.emplace(sink_tree_node);
        tree_node_queue.push(sink_tree_node);

        // plot wire.
        feed << "PATH" << std::endl;
        feed << "LAYER 2" << std::endl;
        feed << "DATATYPE 0" << std::endl;
        feed << "WIDTH " + std::to_string(wire_width) << std::endl;
        feed << "XY" << std::endl;
        feed << source_tree_node->get_point().get_x() << " : " << source_tree_node->get_point().get_y() << std::endl;
        feed << sink_tree_node->get_point().get_x() << " : " << sink_tree_node->get_point().get_y() << std::endl;
        feed << "ENDEL" << std::endl;
      }
      tree_node_queue.pop();
    }
  }
  feed << "ENDSTR" << std::endl;
  feed << "ENDLIB" << std::endl;
  file_stream << feed.str();
  feed.clear();
  file_stream.close();
}

void iPL_API::plotModuleListForDebug(std::vector<std::string> module_prefix_list, std::string path)
{
  std::ofstream file_stream;
  file_stream.open(path);
  if (!file_stream.good()) {
    LOG_WARNING << "Cannot open file for module list !";
  }

  auto* ipl_layout = PlacerDBInst.get_layout();
  auto* ipl_design = PlacerDBInst.get_design();
  auto* topo_manager = PlacerDBInst.get_topo_manager();

  std::map<std::string, std::vector<Instance*>> inst_list_map;
  for (std::string prefix_name : module_prefix_list) {
    inst_list_map.emplace(prefix_name, std::vector<Instance*>{});
  }

  for (auto* inst : ipl_design->get_instance_list()) {
    std::string inst_name = inst->get_name();
    for (auto pair : inst_list_map) {
      std::string prefix_name = pair.first;
      if (inst_name.size() < prefix_name.size()) {
        continue;
      }
      if (inst_name.substr(0, prefix_name.size()) == prefix_name) {
        inst_list_map[prefix_name].push_back(inst);
      }
    }
  }

  SteinerWirelength stwl_eval(PlacerDBInst.get_topo_manager());
  stwl_eval.updateAllNetWorkPointPair();

  std::stringstream feed;
  feed << "HEADER 600" << std::endl;
  feed << "BGNLIB" << std::endl;
  feed << "LIBNAME ITDP_LIB" << std::endl;
  feed << "UNITS 0.001 1e-9" << std::endl;
  feed << "BGNSTR" << std::endl;
  feed << "STRNAME core" << std::endl;
  feed << "BOUNDARY" << std::endl;
  feed << "LAYER 0" << std::endl;
  feed << "DATATYPE 0" << std::endl;
  feed << "XY" << std::endl;

  auto core_shape = ipl_layout->get_core_shape();
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << "ENDEL" << std::endl;

  // print all instances.
  for (auto* inst : ipl_design->get_instance_list()) {
    feed << "BOUNDARY" << std::endl;
    feed << "LAYER 0" << std::endl;
    feed << "DATATYPE 0" << std::endl;
    feed << "XY" << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << "ENDEL" << std::endl;
  }

  // print specify instances and store unrelative nets.
  int32_t layer_idx = 1;
  for (auto pair : inst_list_map) {
    auto& inst_list = pair.second;
    std::set<NetWork*> relative_nets;
    std::set<NetWork*> unrelative_nets;
    for (auto* inst : inst_list) {
      auto* group = topo_manager->findGroup(inst->get_name());
      for (auto* node : group->get_node_list()) {
        auto* network = node->get_network();
        bool skip_flag = false;
        for (auto* network_node : network->get_node_list()) {
          // skip the origin node.
          if (network_node == node) {
            continue;
          }
          // same type instance.
          auto* node_group = network_node->get_group();
          if (node_group) {
            std::string node_group_name = node_group->get_name();
            if ((node_group_name.size() >= pair.first.size()) && (node_group_name.substr(0, pair.first.size()) == pair.first)) {
              relative_nets.emplace(network);
              skip_flag = true;
              continue;
            }
          }
          if (skip_flag == true) {
            break;
          }

          unrelative_nets.emplace(network);
        }
      }

      feed << "BOUNDARY" << std::endl;
      feed << "LAYER " << layer_idx << std::endl;
      feed << "DATATYPE 0" << std::endl;
      feed << "XY" << std::endl;
      feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
      feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
      feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
      feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
      feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
      // feed << "STRING " << pair.first << std::endl;
      feed << "ENDEL" << std::endl;
    }

    // print relative nets.
    layer_idx++;
    int64_t relative_wl = 0;
    for (auto* network : relative_nets) {
      // skip the clock net.
      if (this->isClockNet(network->get_name())) {
        continue;
      }

      relative_wl += stwl_eval.obtainNetWirelength(network->get_name());
      auto* multi_tree = stwl_eval.obtainMultiTree(network);
      std::set<TreeNode*> visited_tree_nodes;
      std::queue<TreeNode*> tree_node_queue;
      tree_node_queue.push(multi_tree->get_root());
      while (!tree_node_queue.empty()) {
        auto* source_tree_node = tree_node_queue.front();
        if (visited_tree_nodes.find(source_tree_node) == visited_tree_nodes.end()) {
          // plotPinName(feed, network->get_name(), source_tree_node);
          visited_tree_nodes.emplace(source_tree_node);
        }
        for (auto* sink_tree_node : source_tree_node->get_child_list()) {
          // plotPinName(feed, network->get_name(), sink_tree_node);
          visited_tree_nodes.emplace(sink_tree_node);
          tree_node_queue.push(sink_tree_node);

          // plot wire.
          feed << "PATH" << std::endl;
          feed << "LAYER " << layer_idx << std::endl;
          feed << "DATATYPE 0" << std::endl;
          feed << "WIDTH "
               << "160" << std::endl;
          feed << "XY" << std::endl;
          feed << source_tree_node->get_point().get_x() << " : " << source_tree_node->get_point().get_y() << std::endl;
          feed << sink_tree_node->get_point().get_x() << " : " << sink_tree_node->get_point().get_y() << std::endl;
          feed << "ENDEL" << std::endl;
        }
        tree_node_queue.pop();
      }
    }

    // print unrelative nets.
    layer_idx++;
    int64_t unrelative_wl = 0;
    for (auto* network : unrelative_nets) {
      // skip the clock net.
      if (this->isClockNet(network->get_name())) {
        continue;
      }

      unrelative_wl += stwl_eval.obtainNetWirelength(network->get_name());
      auto* multi_tree = stwl_eval.obtainMultiTree(network);
      std::set<TreeNode*> visited_tree_nodes;
      std::queue<TreeNode*> tree_node_queue;
      tree_node_queue.push(multi_tree->get_root());
      while (!tree_node_queue.empty()) {
        auto* source_tree_node = tree_node_queue.front();
        if (visited_tree_nodes.find(source_tree_node) == visited_tree_nodes.end()) {
          // plotPinName(feed, network->get_name(), source_tree_node);
          visited_tree_nodes.emplace(source_tree_node);
        }
        for (auto* sink_tree_node : source_tree_node->get_child_list()) {
          // plotPinName(feed, network->get_name(), sink_tree_node);
          visited_tree_nodes.emplace(sink_tree_node);
          tree_node_queue.push(sink_tree_node);

          // plot wire.
          feed << "PATH" << std::endl;
          feed << "LAYER " << layer_idx << std::endl;
          feed << "DATATYPE 0" << std::endl;
          feed << "WIDTH "
               << "160" << std::endl;
          feed << "XY" << std::endl;
          feed << source_tree_node->get_point().get_x() << " : " << source_tree_node->get_point().get_y() << std::endl;
          feed << sink_tree_node->get_point().get_x() << " : " << sink_tree_node->get_point().get_y() << std::endl;
          feed << "ENDEL" << std::endl;
        }
        tree_node_queue.pop();
      }
    }

    // print relative net wirelength.
    LOG_INFO << "MOUDULE: " << pair.first << " Relative Net WL : " << relative_wl;

    // print unrelative net wirelength.
    LOG_INFO << "MOUDULE: " << pair.first << " Unrelative Net WL : " << unrelative_wl;
    layer_idx++;
  }

  feed << "ENDSTR" << std::endl;
  feed << "ENDLIB" << std::endl;
  file_stream << feed.str();
  feed.clear();
  file_stream.close();
}

void iPL_API::plotModuleStateForDebug(std::vector<std::string> special_inst_list, std::string path)
{
  std::ofstream file_stream;
  file_stream.open(path);
  if (!file_stream.good()) {
    LOG_WARNING << "Cannot open file for module list !";
  }

  auto* ipl_layout = PlacerDBInst.get_layout();
  auto* ipl_design = PlacerDBInst.get_design();

  std::vector<Instance*> special_insts;
  for (std::string name : special_inst_list) {
    auto* inst = ipl_design->find_instance(name);
    special_insts.push_back(inst);
  }

  std::stringstream feed;
  feed << "HEADER 600" << std::endl;
  feed << "BGNLIB" << std::endl;
  feed << "LIBNAME ITDP_LIB" << std::endl;
  feed << "UNITS 0.001 1e-9" << std::endl;
  feed << "BGNSTR" << std::endl;
  feed << "STRNAME core" << std::endl;
  feed << "BOUNDARY" << std::endl;
  feed << "LAYER 0" << std::endl;
  feed << "DATATYPE 0" << std::endl;
  feed << "XY" << std::endl;

  auto core_shape = ipl_layout->get_core_shape();
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << core_shape.get_ur_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ur_y() << std::endl;
  feed << core_shape.get_ll_x() << " : " << core_shape.get_ll_y() << std::endl;
  feed << "ENDEL" << std::endl;

  // print all instances.
  for (auto* inst : ipl_design->get_instance_list()) {
    feed << "BOUNDARY" << std::endl;
    feed << "LAYER 0" << std::endl;
    feed << "DATATYPE 0" << std::endl;
    feed << "XY" << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << "ENDEL" << std::endl;
  }

  // print special instances.
  for (auto* inst : special_insts) {
    feed << "BOUNDARY" << std::endl;
    feed << "LAYER 1" << std::endl;
    feed << "DATATYPE 0" << std::endl;
    feed << "XY" << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << inst->get_shape().get_ur_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ur_y() << std::endl;
    feed << inst->get_shape().get_ll_x() << " : " << inst->get_shape().get_ll_y() << std::endl;
    feed << "ENDEL" << std::endl;
  }

  feed << "ENDSTR" << std::endl;
  feed << "ENDLIB" << std::endl;
  file_stream << feed.str();
  feed.clear();
  file_stream.close();
}

void iPL_API::saveNetPinInfoForDebug(std::string path)
{
  std::ofstream file_stream;
  file_stream.open(path);
  if (!file_stream.good()) {
    LOG_WARNING << "Cannot open file for net pin list info !";
  }

  std::vector<std::string> pin_name_list;
  for (auto* net : PlacerDBInst.get_design()->get_net_list()) {
    for (auto* pin : net->get_pins()) {
      pin_name_list.push_back(pin->get_name());
    }
  }

  std::sort(pin_name_list.begin(), pin_name_list.end());

  file_stream << "There are pins count : " << pin_name_list.size() + 1 << std::endl;
  file_stream << std::endl;

  for (std::string pin_name : pin_name_list) {
    file_stream << pin_name << std::endl;
  }
  file_stream.close();
}

void iPL_API::savePinListInfoForDebug(std::string path)
{
  std::ofstream file_stream;
  file_stream.open(path);
  if (!file_stream.good()) {
    LOG_WARNING << "Cannot open file for pin list info !";
  }

  std::vector<std::string> pin_name_list;
  for (auto* pin : PlacerDBInst.get_design()->get_pin_list()) {
    pin_name_list.push_back(pin->get_name());
  }

  std::sort(pin_name_list.begin(), pin_name_list.end());

  file_stream << "There are pins count : " << pin_name_list.size() + 1 << std::endl;
  file_stream << std::endl;

  for (std::string pin_name : pin_name_list) {
    file_stream << pin_name << std::endl;
  }
  file_stream.close();
}

void iPL_API::reportSTWLInfo(std::ofstream& feed)
{
  auto* topo_manager = PlacerDBInst.get_topo_manager();
  SteinerWirelength stwl_eval(topo_manager);
  stwl_eval.updateAllNetWorkPointPair();

  int64_t sum_stwl = 0;
  int64_t max_stwl = 0;

  for (auto* network : topo_manager->get_network_list()) {
    int64_t stwl = stwl_eval.obtainNetWirelength(network->get_name());
    stwl > max_stwl ? max_stwl = stwl : max_stwl;
    sum_stwl += stwl;
  }

  feed << "Total STWL : " << sum_stwl << std::endl;
  feed << "Max STWL : " << max_stwl << std::endl;
  feed << std::endl;
}

void iPL_API::printHPWLInfo()
{
  auto* topo_manager = PlacerDBInst.get_topo_manager();
  HPWirelength hpwl_eval(topo_manager);
  LOG_INFO << "Current Stage Total HPWL: " << hpwl_eval.obtainTotalWirelength();
}

void iPL_API::reportHPWLInfo(std::ofstream& feed)
{
  auto* topo_manager = PlacerDBInst.get_topo_manager();
  HPWirelength hpwl_eval(topo_manager);
  feed << "Total HPWL: " << hpwl_eval.obtainTotalWirelength() << std::endl;
  feed << std::endl;
}

void iPL_API::reportLongNetInfo(std::ofstream& feed)
{
  auto* pl_config = PlacerDBInst.get_placer_config();
  int32_t max_wirelength_constraint = pl_config->get_buffer_config().get_max_wirelength_constraint();
  feed << "Report LongNet HPWL Exceed " << max_wirelength_constraint << std::endl;

  auto core_shape = PlacerDBInst.get_layout()->get_core_shape();
  int32_t core_width = core_shape.get_width();
  int32_t core_height = core_shape.get_height();
  int32_t long_width = max_wirelength_constraint;
  int32_t long_height = max_wirelength_constraint;
  auto* topo_manager = PlacerDBInst.get_topo_manager();

  int net_cnt = 0;
  for (auto* network : topo_manager->get_network_list()) {
    if (fabs(network->get_net_weight()) < 1e-7) {
      continue;
    }
    if (PlacerDBInst.get_placer_config()->isTimingAwareMode()) {
      if (this->isClockNet(network->get_name())) {
        continue;
      }
    }

    auto shape = network->obtainNetWorkShape();
    int32_t network_width = shape.get_width();
    int32_t network_height = shape.get_height();

    if (network_width > long_width && network_height > long_height) {
      feed << "Net : " << network->get_name() << " Width/CoreWidth " << network_width << "/" << core_width << " Height/CoreHeight "
           << network_height << "/" << core_height << std::endl;
      ++net_cnt;
    } else if (network_width > long_width) {
      feed << "Net : " << network->get_name() << " Width/CoreWidth " << network_width << "/" << core_width << std::endl;
      ++net_cnt;
    } else if (network_height > long_height) {
      feed << "Net : " << network->get_name() << " Height/CoreHeight " << network_height << "/" << core_height << std::endl;
      ++net_cnt;
    }
  }

  feed << std::endl;
  feed << "SUMMARY : "
       << "AcrossLongNets / Total Nets = " << net_cnt << " / " << topo_manager->get_network_list().size() << std::endl;
  feed << std::endl;
}

void iPL_API::reportOverlapInfo(std::ofstream& feed)
{
  LayoutChecker* checker = new LayoutChecker(&PlacerDBInst);

  feed << "Overlap Violation" << std::endl;
  feed << "Regions In the Layout : " << std::endl;
  int32_t idx = 0;
  for (auto* region : PlacerDBInst.get_design()->get_region_list()) {
    feed << "Region " << idx++ << region->get_name() << std::endl;
    for (auto boundary : region->get_boundaries()) {
      feed << "Boundary : " << boundary.get_ll_x() << "," << boundary.get_ll_y() << " " << boundary.get_ur_x() << "," << boundary.get_ur_y()
           << std::endl;
    }
  }

  feed << std::endl;

  std::vector<std::vector<Instance*>> clique_list = checker->obtainOverlapInstClique();
  feed << "Illegal Overlap Instance Cliques Count : " << clique_list.size() << std::endl;
  for (size_t i = 0; i < clique_list.size(); i++) {
    feed << "Overlap Clique : " << i << std::endl;
    for (size_t j = 0; j < clique_list.at(i).size(); j++) {
      auto* inst = clique_list.at(i).at(j);
      feed << "Inst " << inst->get_name() << " Coordinate : " << inst->get_shape().get_ll_x() << "," << inst->get_shape().get_ll_y() << " "
           << inst->get_shape().get_ur_x() << "," << inst->get_shape().get_ur_y() << std::endl;
      if (clique_list.at(i).size() == 1) {
        feed << "Maybe overlap with blockage !" << std::endl;
      }
    }
  }

  delete checker;
}

void iPL_API::reportTimingInfo(std::ofstream& feed)
{
  if (this->isSTAStarted()) {
    iPLAPIInst.initTimingEval();
    iPLAPIInst.updateTiming();

    for (std::string clock_name : iPLAPIInst.obtainClockNameList()) {
      double wns = iPLAPIInst.obtainWNS(clock_name.c_str(), ista::AnalysisMode::kMax);
      double tns = iPLAPIInst.obtainTNS(clock_name.c_str(), ista::AnalysisMode::kMax);
      feed << "Clock : " << clock_name << " WNS : " << wns << ";"
           << " TNS : " << tns << std::endl;
    }
    feed << std::endl;

    iPLAPIInst.destroyTimingEval();
  }
}

void iPL_API::reportCongestionInfo()
{
  std::vector<float> gr_congestion = iPLAPIInst.evalGRCong();

  auto report_tbl = std::make_unique<ieda::ReportTable>("table");
  (*report_tbl)[0][0].set_cell_bg_color(fort::color::red);
  (*report_tbl).column(0).set_cell_text_align(fort::text_align::center);
  (*report_tbl) << TABLE_HEAD;
  (*report_tbl)[0][0] = "Congestion Info";
  (*report_tbl)[1][0] = "ACE";
  (*report_tbl)[1][1] = "TOF";
  (*report_tbl)[1][2] = "MOF";
  (*report_tbl) << TABLE_ENDLINE;
  (*report_tbl)[1][0] = std::to_string(gr_congestion[0]);
  (*report_tbl)[1][1] = std::to_string(gr_congestion[1]);
  (*report_tbl)[1][2] = std::to_string(gr_congestion[2]);
  (*report_tbl) << TABLE_ENDLINE;
  std::cout << (*report_tbl).to_string() << std::endl;

  std::string plot_path = "./result/pl/report/";
  std::string output_file_name = "CongMap";
  iPLAPIInst.plotCongMap(plot_path, output_file_name);
  iPLAPIInst.destroyCongEval();
}

}  // namespace ipl
