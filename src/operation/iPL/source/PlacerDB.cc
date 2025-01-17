/*
 * @Author: S.J Chen
 * @Date: 2022-02-18 11:24:20
 * @LastEditTime: 2023-02-22 11:34:09
 * @LastEditors: Shijian Chen  chenshj@pcl.ac.cn
 * @Description:
 * @FilePath: /irefactor/src/operation/iPL/source/PlacerDB.cc
 * Contact : https://github.com/sjchanson
 */

#include "PlacerDB.hh"

#include "Log.hh"
#include "data/Rectangle.hh"

namespace ipl {

PlacerDB& PlacerDB::getInst()
{
  if (!_placer_db_instance) {
    _placer_db_instance = new PlacerDB();
  }

  return *_placer_db_instance;
}

void PlacerDB::destoryInst()
{
  if (_placer_db_instance) {
    delete _placer_db_instance;
  }
}

PlacerDB::PlacerDB() : _config(nullptr), _db_wrapper(nullptr), _topo_manager(nullptr), _grid_manager(nullptr)
{
}

PlacerDB::~PlacerDB()
{
  if (_config) {
    delete _config;
  }
  if (_db_wrapper) {
    delete _db_wrapper;
  }
  if (_topo_manager) {
    delete _topo_manager;
  }
  if (_grid_manager) {
    delete _grid_manager;
  }
}

void PlacerDB::initPlacerDB(std::string pl_json_path, DBWrapper* db_wrapper)
{
  _db_wrapper = db_wrapper;
  updatePlacerConfig(pl_json_path);
  sortDataForParallel();
  initTopoManager();
  initGridManager();
  updateTopoManager();
  updateGridManager();
  // updateFromSourceDataBase();
  adaptTargetDensity();
  printPlacerDB();
}

void PlacerDB::updatePlacerConfig(std::string pl_json_path)
{
  if (_config) {
    delete _config;
  }
  _config = new Config(pl_json_path);
}

void PlacerDB::initTopoManager()
{
  if (_topo_manager) {
    delete _topo_manager;
  }

  _topo_manager = new TopologyManager();
  auto* pl_design = this->get_design();
  for (auto* pin : pl_design->get_pin_list()) {
    Node* node = new Node(pin->get_name());
    node->set_location(std::move(pin->get_center_coordi()));
    _topo_manager->add_node(node->get_name(), node);
  }

  for (auto* net : pl_design->get_net_list()) {
    NetWork* network = new NetWork(net->get_name());

    network->set_net_weight(net->get_net_weight());

    Pin* driver_pin = net->get_driver_pin();
    if (driver_pin) {
      Node* transmitter = _topo_manager->findNode(driver_pin->get_name());
      transmitter->set_network(network);
      network->set_transmitter(transmitter);
    }

    for (auto* loader_pin : net->get_sink_pins()) {
      Node* receiver = _topo_manager->findNode(loader_pin->get_name());
      receiver->set_network(network);
      network->add_receiver(receiver);
    }

    _topo_manager->add_network(network->get_name(), network);
  }

  for (auto* inst : pl_design->get_instance_list()) {
    Group* group = new Group(inst->get_name());

    for (auto* pin : inst->get_pins()) {
      Node* node = _topo_manager->findNode(pin->get_name());
      node->set_group(group);
      group->add_node(node);
    }

    _topo_manager->add_group(group->get_name(), group);
  }
}

void PlacerDB::initGridManager()
{
  Rectangle<int32_t> core_shape = this->get_layout()->get_core_shape();
  int32_t row_height = this->get_layout()->get_row_height();
  int32_t site_width = this->get_layout()->get_site_width();

  _grid_manager = new GridManager(core_shape, core_shape.get_width() / site_width, core_shape.get_height() / row_height, 1.0);
  initGridManagerFixedArea();
}

void PlacerDB::initGridManagerFixedArea()
{
  if (!_grid_manager) {
    LOG_WARNING << "grid manager has not been initialized ! ";
    return;
  }

  for (auto* inst : this->get_design()->get_instance_list()) {
    if (inst->isOutsideInstance()) {
      continue;
    }

    if (!inst->isFixed()) {
      continue;
    }

    // add fix insts.
    std::vector<Grid*> overlap_grid_list;
    _grid_manager->obtainOverlapGridList(overlap_grid_list, inst->get_shape());
    for (auto* grid : overlap_grid_list) {
      int64_t overlap_area = _grid_manager->obtainOverlapArea(grid, inst->get_shape());
      grid->add_fixed_area(overlap_area);
    }
  }

  // add blockage.
  auto region_list = this->get_design()->get_region_list();
  for (auto* region : region_list) {
    if (region->isFence()) {
      std::vector<Grid*> overlap_grid_list;
      for (auto boundary : region->get_boundaries()) {
        _grid_manager->obtainOverlapGridList(overlap_grid_list, boundary);
        for (auto* grid : overlap_grid_list) {
          // tmp fix overlap area between fixed inst and blockage.
          if (grid->get_fixed_area() != 0) {
            continue;
          }
          int64_t overlap_area = _grid_manager->obtainOverlapArea(grid, boundary);
          grid->add_fixed_area(overlap_area);
        }
      }
    }
  }
}

void PlacerDB::sortDataForParallel()
{
  auto* design = this->get_design();
  design->sortDataForParallel();
  initIgnoreNets(_config->get_ignore_net_degree());
}

void PlacerDB::initIgnoreNets(int32_t ignore_net_degree)
{
  for (auto* net : this->get_design()->get_net_list()) {
    int32_t net_degree = net->get_pins().size();

    if (net_degree < 2 || net_degree > ignore_net_degree) {
      net->set_net_state(NET_STATE::kDontCare);
      net->set_netweight(0.0f);
    }
  }
}

void PlacerDB::updateTopoManager()
{
  for (auto* pin : this->get_design()->get_pin_list()) {
    auto* node = _topo_manager->findNode(pin->get_name());
    node->set_location(std::move(pin->get_center_coordi()));
  }
}

void PlacerDB::updateGridManager()
{
  _grid_manager->clearAllOccupiedArea();

  for (auto* inst : this->get_design()->get_instance_list()) {
    if (inst->isOutsideInstance()) {
      continue;
    }

    if (inst->get_coordi().isUnLegal()) {
      continue;
    }

    if (inst->isFixed()) {  // Fixed area has been already add.
      continue;
    }

    auto inst_shape = std::move(inst->get_shape());
    std::vector<Grid*> overlap_grid_list;
    _grid_manager->obtainOverlapGridList(overlap_grid_list, inst_shape);
    for (auto* grid : overlap_grid_list) {
      int64_t overlap_area = _grid_manager->obtainOverlapArea(grid, inst_shape);
      grid->add_area(overlap_area);
    }
  }
}

void PlacerDB::updateFromSourceDataBase()
{
  _db_wrapper->updateFromSourceDataBase();
  initTopoManager();
  updateTopoManager();
  updateGridManager();
}

void PlacerDB::updateFromSourceDataBase(std::vector<std::string> inst_list)
{
  _db_wrapper->updateFromSourceDataBase(inst_list);
}

void PlacerDB::updateInstancesForDebug(std::vector<Instance*> inst_list)
{
  auto* design = this->get_design();
  for (auto* inst : inst_list) {
    design->add_instance(inst);
  }
}

void PlacerDB::printPlacerDB() const
{
  printLayoutInfo();
  printInstanceInfo();
  printNetInfo();
  printPinInfo();
  printRegionInfo();
}

void PlacerDB::printLayoutInfo() const
{
  Design* design = this->get_design();
  const Layout* layout = this->get_layout();

  std::string design_name = design->get_design_name();
  int32_t database_unit = layout->get_database_unit();
  Rectangle<int32_t> die_rect = layout->get_die_shape();
  Rectangle<int32_t> core_rect = layout->get_core_shape();
  int32_t row_height = layout->get_row_height();
  int32_t site_width = layout->get_site_width();

  LOG_INFO << "Design name : " << design_name;
  LOG_INFO << "Database unit : " << database_unit;
  LOG_INFO << "Die rectangle : " << die_rect.get_ll_x() << "," << die_rect.get_ll_y() << " " << die_rect.get_ur_x() << ","
           << die_rect.get_ur_y();
  LOG_INFO << "Core rectangle : " << core_rect.get_ll_x() << "," << core_rect.get_ll_y() << " " << core_rect.get_ur_x() << ","
           << core_rect.get_ur_y();
  LOG_INFO << "Row height : " << row_height;
  LOG_INFO << "Site width : " << site_width;

  int64_t core_area = static_cast<int64_t>(core_rect.get_width()) * static_cast<int64_t>(core_rect.get_height());
  int64_t place_instance_area = 0;
  int64_t non_place_instance_area = 0;

  for (auto* inst : design->get_instance_list()) {
    // Ignore the insts outside the core.
    if (inst->isOutsideInstance() && inst->isFixed()) {
      continue;
    }

    // if (inst->get_cell_master() && inst->get_cell_master()->isIOCell()) {
    //   continue;
    // }
    // for ispd's benchmark
    // if (inst->isOutsideInstance()) {
    //   continue;
    // }

    int64_t inst_width = static_cast<int64_t>(inst->get_shape().get_width());
    int64_t inst_height = static_cast<int64_t>(inst->get_shape().get_height());
    if (inst->isFixed()) {
      non_place_instance_area += inst_width * inst_height;
    } else {
      place_instance_area += inst_width * inst_height;
    }
  }

  // TODO : exclude the overlap region.
  for (auto* blockage : design->get_region_list()) {
    for (auto boundary : blockage->get_boundaries()) {
      int64_t boundary_width = static_cast<int64_t>(boundary.get_width());
      int64_t boundary_height = static_cast<int64_t>(boundary.get_height());
      non_place_instance_area += boundary_width * boundary_height;
    }
  }

  LOG_INFO << "Core area : " << core_area;
  LOG_INFO << "Non place instance area : " << non_place_instance_area;
  LOG_INFO << "Place instance area : " << place_instance_area;

  double util = static_cast<double>(place_instance_area) / (core_area - non_place_instance_area) * 100;
  LOG_INFO << "Uitization(%) : " << util;
  LOG_ERROR_IF(util > 100.1) << "Utilization exceeds 100%";
}

float PlacerDB::obtainUtilization()
{
  Design* design = this->get_design();
  const Layout* layout = this->get_layout();
  Rectangle<int32_t> core_rect = layout->get_core_shape();
  int64_t core_area = static_cast<int64_t>(core_rect.get_width()) * static_cast<int64_t>(core_rect.get_height());
  int64_t place_instance_area = 0;
  int64_t non_place_instance_area = 0;

  for (auto* inst : design->get_instance_list()) {
    // Ignore the insts outside the core.
    if (inst->isOutsideInstance() && inst->isFixed()) {
      continue;
    }
    int64_t inst_width = static_cast<int64_t>(inst->get_shape().get_width());
    int64_t inst_height = static_cast<int64_t>(inst->get_shape().get_height());
    if (inst->isFixed()) {
      non_place_instance_area += inst_width * inst_height;
    } else {
      place_instance_area += inst_width * inst_height;
    }
  }

  // TODO : exclude the overlap region.
  for (auto* blockage : design->get_region_list()) {
    for (auto boundary : blockage->get_boundaries()) {
      int64_t boundary_width = static_cast<int64_t>(boundary.get_width());
      int64_t boundary_height = static_cast<int64_t>(boundary.get_height());
      non_place_instance_area += boundary_width * boundary_height;
    }
  }

  float util = static_cast<float>(place_instance_area) / (core_area - non_place_instance_area);
  return util;
}

void PlacerDB::adaptTargetDensity()
{
  float cur_util = this->obtainUtilization();
  float user_target_density = this->get_placer_config()->get_nes_config().get_target_density();
  if (user_target_density < cur_util) {
    float setting_util = cur_util + 0.001;
    this->get_placer_config()->get_nes_config().set_target_density(setting_util);
  }
}

void PlacerDB::printInstanceInfo() const
{
  const Layout* layout = this->get_layout();
  Design* design = this->get_design();

  int32_t num_instances = 0;
  int32_t num_macros = 0;
  int32_t num_logic_insts = 0;
  int32_t num_flipflop_cells = 0;
  int32_t num_clock_buffers = 0;
  int32_t num_logic_buffers = 0;
  int32_t num_io_cells = 0;
  int32_t num_physical_insts = 0;
  int32_t num_outside_insts = 0;
  int32_t num_fake_instances = 0;
  int32_t num_unplaced_instances = 0;
  int32_t num_placed_instances = 0;
  int32_t num_fixed_instances = 0;
  int32_t num_cell_masters = static_cast<int32_t>(layout->get_cell_list().size());

  for (auto* inst : design->get_instance_list()) {
    num_instances++;

    Cell* cell_master = inst->get_cell_master();
    if (cell_master) {
      if (cell_master->isMacro()) {
        num_macros++;
      } else if (cell_master->isLogic()) {
        num_logic_insts++;
      } else if (cell_master->isFlipflop()) {
        num_flipflop_cells++;
      } else if (cell_master->isClockBuffer()) {
        num_clock_buffers++;
      } else if (cell_master->isLogicBuffer()) {
        num_logic_buffers++;
      } else if (cell_master->isIOCell()) {
        num_io_cells++;
      } else if (cell_master->isPhysicalFiller()) {
        num_physical_insts++;
      } else {
        LOG_WARNING << "Instance : " + inst->get_name() + " doesn't have a cell type.";
      }
    }

    if (inst->isFakeInstance()) {
      num_fake_instances++;
    } else if (inst->isNormalInstance()) {
      //
    } else if (inst->isOutsideInstance()) {
      num_outside_insts++;
    } else {
      // LOG_WARNING << "Instance : " + inst->get_name() + " doesn't have a instance type.";
    }

    if (inst->isUnPlaced()) {
      num_unplaced_instances++;
    } else if (inst->isPlaced()) {
      num_placed_instances++;
    } else if (inst->isFixed()) {
      num_fixed_instances++;
    } else {
      LOG_WARNING << "Instance : " + inst->get_name() + " doesn't have a instance state.";
    }
  }

  LOG_INFO << "Instances Num : " << num_instances;
  LOG_INFO << "1. Macro Num : " << num_macros;
  LOG_INFO << "2. Stdcell Num : " << num_instances - num_macros;
  LOG_INFO << "2.1 Logic Instances : " << num_logic_insts;
  LOG_INFO << "2.2 Flipflops : " << num_flipflop_cells;
  LOG_INFO << "2.3 Clock Buffers : " << num_clock_buffers;
  LOG_INFO << "2.4 Logic Buffers : " << num_logic_buffers;
  LOG_INFO << "2.5 IO Cells : " << num_io_cells;
  LOG_INFO << "2.6 Physical Instances : " << num_physical_insts;
  LOG_INFO << "Core Outside Instances : " << num_outside_insts;
  LOG_INFO << "Fake Instances : " << num_fake_instances;
  LOG_INFO << "Unplaced Instances Num : " << num_unplaced_instances;
  LOG_INFO << "Placed Instances Num : " << num_placed_instances;
  LOG_INFO << "Fixed Instances Num : " << num_fixed_instances;
  LOG_INFO << "Optional CellMaster Num : " << num_cell_masters;
}

void PlacerDB::printNetInfo() const
{
  Design* design = this->get_design();

  int32_t num_nets = 0;
  int32_t num_clock_nets = 0;
  int32_t num_reset_nets = 0;
  int32_t num_signal_nets = 0;
  int32_t num_fake_nets = 0;
  int32_t num_normal_nets = 0;
  int32_t num_dontcare_nets = 0;

  int32_t num_no_type_nets = 0;
  int32_t num_no_state_nets = 0;

  for (auto* net : design->get_net_list()) {
    num_nets++;
    if (net->isClockNet()) {
      num_clock_nets++;
    } else if (net->isResetNet()) {
      num_reset_nets++;
    } else if (net->isSignalNet()) {
      num_signal_nets++;
    } else if (net->isFakeNet()) {
      num_fake_nets++;
    } else {
      num_no_type_nets++;
    }

    if (net->isNormalStateNet()) {
      num_normal_nets++;
    } else if (net->isDontCareNet()) {
      num_dontcare_nets++;
    } else {
      num_no_state_nets++;
    }
  }

  LOG_INFO << "Nets Num : " << num_nets;
  LOG_INFO << "1. ClockNets Num : " << num_clock_nets;
  LOG_INFO << "2. ResetNets Num : " << num_reset_nets;
  LOG_INFO << "3. SignalNets Num : " << num_signal_nets;
  LOG_INFO << "4. FakeNets Num : " << num_fake_nets;
  LOG_INFO << "Don't Care Net Num : " << num_dontcare_nets;

  if (num_no_type_nets != 0) {
    LOG_WARNING << "Existed Nets don't have NET_TYPE : " << num_no_type_nets;
  }
  if (num_no_state_nets != 0) {
    LOG_WARNING << "Existed Nets don't have NET_STATE : " << num_no_state_nets;
  }
}

void PlacerDB::printPinInfo() const
{
  Design* design = this->get_design();

  int32_t num_pins = 0;
  int32_t num_io_ports = 0;
  int32_t num_instance_ports = 0;
  int32_t num_fake_pins = 0;

  for (auto* pin : design->get_pin_list()) {
    num_pins++;
    if (pin->isIOPort()) {
      num_io_ports++;
    } else if (pin->isInstancePort()) {
      num_instance_ports++;
    } else if (pin->isFakePin()) {
      num_fake_pins++;
    } else {
      LOG_WARNING << "Pin : " + pin->get_name() + " doesn't have a pin type.";
    }
  }

  LOG_INFO << "Pins Num : " << num_pins;
  LOG_INFO << "1. IO Ports Num : " << num_io_ports;
  LOG_INFO << "2. Instance Ports Num : " << num_instance_ports;
  LOG_INFO << "3. Fake Pins Num : " << num_fake_pins;
}

void PlacerDB::printRegionInfo() const
{
  Design* design = this->get_design();

  int32_t num_regions = 0;
  num_regions = design->get_region_list().size();

  LOG_INFO << "Regions Num : " << num_regions;
}

void PlacerDB::saveVerilogForDebug(std::string path)
{
  _db_wrapper->saveVerilogForDebug(path);
}

// private
PlacerDB* PlacerDB::_placer_db_instance = nullptr;

}  // namespace ipl