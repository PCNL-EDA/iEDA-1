#include "Evaluator.h"

#include <fstream>

#include "CTSAPI.hpp"
#include "CtsReport.h"
#include "log/Log.hh"
namespace icts {

void Evaluator::init()
{
  CTSAPIInst.setPropagateClock();
  printLog();
  transferData();
}

void Evaluator::printLog()
{
  LOG_INFO << "\033[1;31m";
  LOG_INFO << R"(                  _             _              )";
  LOG_INFO << R"(                 | |           | |             )";
  LOG_INFO << R"(   _____   ____ _| |_   _  __ _| |_ ___  _ __  )";
  LOG_INFO << R"(  / _ \ \ / / _` | | | | |/ _` | __/ _ \| '__| )";
  LOG_INFO << R"( |  __/\ V / (_| | | |_| | (_| | || (_) | |    )";
  LOG_INFO << R"(  \___| \_/ \__,_|_|\__,_|\__,_|\__\___/|_|    )";
  LOG_INFO << "\033[0m";
  LOG_INFO << "Enter evaluator!";
}

void Evaluator::transferData()
{
  _eval_nets.clear();
  auto* design = CTSAPIInst.get_design();
  auto& clk_nets = design->get_nets();
  for (auto* clk_net : clk_nets) {
    _eval_nets.emplace_back(EvalNet(clk_net));
  }
}

void Evaluator::evaluate()
{
  CTSAPIInst.refresh();
  for (auto eval_net : _eval_nets) {
    CTSAPIInst.buildRCTree(eval_net);
  }
  CTSAPIInst.reportTiming();
}

void Evaluator::statistics(const std::string& save_dir) const
{
  auto* config = CTSAPIInst.get_config();
  auto dir = save_dir == "" ? config->get_sta_workspace() : save_dir + "/statistics";
  // wirelength statistics(type: total, top, trunk, leaf, total certer dist,
  // max)
  auto wl_rpt = CtsReportTable::createReportTable("Wire length stats", CtsReportType::kWIRE_LENGTH);
  auto hpwl_wl_rpt = CtsReportTable::createReportTable("HPWL Wire length stats", CtsReportType::kHP_WIRE_LENGTH);
  std::map<std::string, int> cell_count_map;
  double top_wire_len = 0.0;
  double trunk_wire_len = 0.0;
  double leaf_wire_len = 0.0;
  double total_wire_len = 0.0;
  double max_net_len = 0.0;
  double hpwl_top_wire_len = 0.0;
  double hpwl_trunk_wire_len = 0.0;
  double hpwl_leaf_wire_len = 0.0;
  double hpwl_total_wire_len = 0.0;
  double hpwl_max_net_len = 0.0;
  for (const auto& eval_net : _eval_nets) {
    // wire length
    auto net_len = eval_net.getWireLength();
    auto hpwl_net_len = eval_net.getHPWL();
    auto type = eval_net.netType();
    switch (type) {
      case NetType::kTop:
        top_wire_len += net_len;
        hpwl_top_wire_len += hpwl_net_len;
        break;
      case NetType::kTrunk:
        trunk_wire_len += net_len;
        hpwl_trunk_wire_len += hpwl_net_len;
        break;
      case NetType::kLeaf:
        leaf_wire_len += net_len;
        hpwl_leaf_wire_len += hpwl_net_len;
        break;
      default:
        break;
    }
    total_wire_len += net_len;
    hpwl_total_wire_len += hpwl_net_len;
    max_net_len = std::max(max_net_len, net_len);
    hpwl_max_net_len = std::max(hpwl_max_net_len, hpwl_net_len);
    if (eval_net.is_newly()) {
      // cell count
      auto cell_master = eval_net.get_driver()->get_cell_master();
      if (cell_count_map.count(cell_master) == 0) {
        cell_count_map[cell_master] = 1;
      } else {
        cell_count_map[cell_master]++;
      }
    }
  }
  (*wl_rpt) << "Top" << Str::printf("%.3f", top_wire_len) << TABLE_ENDLINE;
  (*wl_rpt) << "Trunk" << Str::printf("%.3f", trunk_wire_len) << TABLE_ENDLINE;
  (*wl_rpt) << "Leaf" << Str::printf("%.3f", leaf_wire_len) << TABLE_ENDLINE;
  (*wl_rpt) << "Total" << Str::printf("%.3f", total_wire_len) << TABLE_ENDLINE;
  (*wl_rpt) << "Max net length" << Str::printf("%.3f", max_net_len) << TABLE_ENDLINE;

  (*hpwl_wl_rpt) << "Top" << Str::printf("%.3f", hpwl_top_wire_len) << TABLE_ENDLINE;
  (*hpwl_wl_rpt) << "Trunk" << Str::printf("%.3f", hpwl_trunk_wire_len) << TABLE_ENDLINE;
  (*hpwl_wl_rpt) << "Leaf" << Str::printf("%.3f", hpwl_leaf_wire_len) << TABLE_ENDLINE;
  (*hpwl_wl_rpt) << "Total" << Str::printf("%.3f", hpwl_total_wire_len) << TABLE_ENDLINE;
  (*hpwl_wl_rpt) << "Max net length" << Str::printf("%.3f", hpwl_max_net_len) << TABLE_ENDLINE;

  auto wl_save_path = dir + "/wire_length.rpt";
  CTSAPIInst.checkFile(dir, "wire_length");
  std::ofstream wl_save_file(wl_save_path);
  wl_save_file << "Generate the report at " << Time::getNowWallTime() << std::endl;
  wl_save_file << wl_rpt->c_str() << "\n\n";
  wl_save_file << hpwl_wl_rpt->c_str();
  // depth statistics(level, num) need to build tree
  // TBD

  // cell stats(Cell type, Count, Area, Capacitance)
  auto cell_stats_rpt = CtsReportTable::createReportTable("Cell stats", CtsReportType::kCELL_STATS);
  struct CellStatsProperty
  {
    int total_num;
    double total_area;
    double total_cap;
  };
  std::map<std::string, CellStatsProperty> cell_property_map;
  for (auto [cell_master, count] : cell_count_map) {
    auto cell_type = CTSAPIInst.getCellType(cell_master);
    auto cell_area = CTSAPIInst.getCellArea(cell_master);
    auto cell_cap = CTSAPIInst.getCellCap(cell_master);
    if (cell_property_map.count(cell_type) == 0) {
      cell_property_map[cell_type] = {count, cell_cap * count};
    } else {
      cell_property_map[cell_type].total_num += count;
      cell_property_map[cell_type].total_area += cell_area * count;
      cell_property_map[cell_type].total_cap += cell_cap * count;
    }
  }
  for (auto [type, cell_property] : cell_property_map) {
    (*cell_stats_rpt) << type << cell_property.total_num << cell_property.total_area << cell_property.total_cap << TABLE_ENDLINE;
  }
  auto cell_stats_save_path = dir + "/cell_stats.rpt";
  CTSAPIInst.checkFile(dir, "cell_stats");
  std::ofstream cell_stats_save_file(cell_stats_save_path);
  cell_stats_save_file << "Generate the report at " << Time::getNowWallTime() << std::endl;
  cell_stats_save_file << cell_stats_rpt->c_str();

  // lib cell distribution(Name, Type, Inst Count, Inst Area)
  auto lib_cell_dist_rpt = CtsReportTable::createReportTable("Library cell distribution", CtsReportType::kLIB_CELL_DIST);
  for (auto [cell_master, count] : cell_count_map) {
    (*lib_cell_dist_rpt) << cell_master << CTSAPIInst.getCellType(cell_master) << count << count * CTSAPIInst.getCellArea(cell_master)
                         << TABLE_ENDLINE;
  }
  auto lib_cell_dist_save_path = dir + "/lib_cell_dist.rpt";
  CTSAPIInst.checkFile(dir, "lib_cell_dist");
  std::ofstream lib_cell_dist_save_file(lib_cell_dist_save_path);
  lib_cell_dist_save_file << "Generate the report at " << Time::getNowWallTime() << std::endl;
  lib_cell_dist_save_file << lib_cell_dist_rpt->c_str();
}

int64_t Evaluator::wireLength() const
{
  int64_t total_wire_len = 0.0;
  for (const auto& eval_net : _eval_nets) {
    int64_t net_wire_len = 0;
    auto signal_wires = eval_net.get_signal_wires();
    for (auto& signal_wire : signal_wires) {
      net_wire_len += signal_wire.getWireLength();
    }
    total_wire_len += net_wire_len;
  }
  LOG_INFO << "Total wire length: " << total_wire_len;
  CTSAPIInst.saveToLog("Total wire length: ", total_wire_len);

  return total_wire_len;
}

void Evaluator::plotPath(const string& inst_name, const string& file) const
{
  auto* config = CTSAPIInst.get_config();
  auto* db_wrapper = CTSAPIInst.get_db_wrapper();
  auto path = config->get_sta_workspace() + "/" + file;
  GDSPloter ploter(path);

  CtsInstance* path_inst = nullptr;
  for (auto& eval_net : _eval_nets) {
    auto inst = eval_net.get_instance(inst_name);
    if (inst && (eval_net.get_driver() != inst)) {
      path_inst = inst;
      break;
    }
  }
  LOG_FATAL_IF(path_inst == nullptr) << "Cannot find instance: " << inst_name;

  ploter.head();
  vector<CtsInstance*> insts;
  while (path_inst) {
    ploter.insertInstance(path_inst);
    insts.emplace_back(path_inst);
    auto before_load_pin = path_inst->get_load_pin();
    if (before_load_pin) {
      auto before_net = before_load_pin->get_net();
      auto driver_pin = before_net->get_driver_pin();
      if (driver_pin->is_io() || !CTSAPIInst.isClockNet(before_net->get_net_name())) {
        break;
      }
      CTSAPIInst.saveToLog("Net: ", before_net->get_net_name());
      CTSAPIInst.saveToLog("Driver Pin: ", before_net->get_driver_pin()->get_full_name());
      for (auto load_pin : before_net->get_load_pins()) {
        if (db_wrapper->ctsToIdb(load_pin)->is_flip_flop_clk()) {
          CTSAPIInst.saveToLog("Load Clock Pin: ", load_pin->get_full_name());
        }
      }
      auto driver_inst = driver_pin->get_instance();
      ploter.insertWire(driver_inst->get_location(), path_inst->get_location());
      path_inst = driver_inst;
    } else {
      break;
    }
  }
  auto core = db_wrapper->get_core_bounding_box();
  ploter.insertPolygon(core, "core", _default_size);
  ploter.strBegin();
  ploter.topBegin();
  for (auto* inst : insts) {
    ploter.refInstance(inst);
  }
  ploter.refPolygon("core");
  ploter.refPolygon("WIRE");
  ploter.strEnd();

  ploter.tail();
  LOG_INFO << "Path to " << inst_name << " has been written to " << path;
}

void Evaluator::plotNet(const string& net_name, const string& file) const
{
  auto* design = CTSAPIInst.get_design();
  auto& clk_nets = design->get_nets();
  CtsNet* net = nullptr;
  for (auto& clk_net : clk_nets) {
    if (clk_net->get_net_name() == net_name) {
      net = clk_net;
      break;
    }
  }
  LOG_FATAL_IF(net == nullptr) << "Net " << net_name << " not found";

  auto* config = CTSAPIInst.get_config();
  auto* db_wrapper = CTSAPIInst.get_db_wrapper();
  auto path = config->get_sta_workspace() + "/" + file;
  GDSPloter ploter(path);

  ploter.head();
  auto insts = net->get_instances();
  for (auto* inst : insts) {
    ploter.insertInstance(inst);
  }
  for (const auto& wire : net->get_signal_wires()) {
    auto first = DataTraits<Endpoint>::getPoint(wire.get_first());
    auto second = DataTraits<Endpoint>::getPoint(wire.get_second());
    ploter.insertWire(first, second);
  }
  auto core = db_wrapper->get_core_bounding_box();
  ploter.insertPolygon(core, "core", _default_size);
  ploter.strBegin();
  ploter.topBegin();
  for (auto* inst : insts) {
    ploter.refInstance(inst);
  }
  ploter.refPolygon("core");
  ploter.refPolygon("WIRE");
  ploter.strEnd();

  ploter.tail();
  LOG_INFO << "Net: " << net_name << " has been written to " << path;
}
}  // namespace icts