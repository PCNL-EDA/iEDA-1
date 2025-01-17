#include "GlobalRouter.hpp"

#include "GDSPlotter.hpp"
#include "RTUtil.hpp"

namespace irt {

// public

void GlobalRouter::initInst(Config& config, Database& database)
{
  if (_gr_instance == nullptr) {
    _gr_instance = new GlobalRouter(config, database);
  }
}

GlobalRouter& GlobalRouter::getInst()
{
  if (_gr_instance == nullptr) {
    LOG_INST.error(Loc::current(), "The instance not initialized!");
  }
  return *_gr_instance;
}

void GlobalRouter::destroyInst()
{
  if (_gr_instance != nullptr) {
    delete _gr_instance;
    _gr_instance = nullptr;
  }
}

void GlobalRouter::route(std::vector<Net>& net_list)
{
  Monitor monitor;

  std::vector<GRNet> gr_net_list = _gr_data_manager.convertToGRNetList(net_list);
  routeGRNetList(gr_net_list);

  LOG_INST.info(Loc::current(), "The ", GetStageName()(Stage::kGlobalRouter), " completed!", monitor.getStatsInfo());
}

// private

GlobalRouter* GlobalRouter::_gr_instance = nullptr;

void GlobalRouter::init(Config& config, Database& database)
{
  _gr_data_manager.input(config, database);
}

void GlobalRouter::routeGRNetList(std::vector<GRNet>& gr_net_list)
{
  GRModel gr_model = initGRModel(gr_net_list);
  buildGRModel(gr_model);
  checkGRModel(gr_model);
  sortGRModel(gr_model);
  routeGRModel(gr_model);
  updateGRModel(gr_model);
  reportGRModel(gr_model);
}

#if 1  // build

GRModel GlobalRouter::initGRModel(std::vector<GRNet>& gr_net_list)
{
  Die& die = _gr_data_manager.getDatabase().get_die();
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();

  GRModel gr_model;
  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  layer_node_map.resize(routing_layer_list.size());
  for (size_t layer_idx = 0; layer_idx < layer_node_map.size(); layer_idx++) {
    GridMap<GRNode>& node_map = layer_node_map[layer_idx];
    node_map.init(die.getXSize(), die.getYSize());
    for (irt_int x = 0; x < die.getXSize(); x++) {
      for (irt_int y = 0; y < die.getYSize(); y++) {
        GRNode& gr_node = node_map[x][y];
        gr_node.set_coord(x, y);
        gr_node.set_layer_idx(static_cast<irt_int>(layer_idx));
      }
    }
  }
  gr_model.set_gr_net_list(gr_net_list);

  return gr_model;
}

void GlobalRouter::buildGRModel(GRModel& gr_model)
{
  buildNeighborMap(gr_model);
  buildNodeSupply(gr_model);
  buildGRNetPriority(gr_model);
}

void GlobalRouter::buildNeighborMap(GRModel& gr_model)
{
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  irt_int bottom_routing_layer_idx = _gr_data_manager.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = _gr_data_manager.getConfig().top_routing_layer_idx;

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  for (irt_int layer_idx = 0; layer_idx < static_cast<irt_int>(layer_node_map.size()); layer_idx++) {
    bool routing_h = routing_layer_list[layer_idx].isPreferH();
    bool routing_v = !routing_h;
    if (layer_idx < bottom_routing_layer_idx || top_routing_layer_idx < layer_idx) {
      routing_h = false;
      routing_v = false;
    }
    GridMap<GRNode>& node_map = layer_node_map[layer_idx];
    for (irt_int x = 0; x < node_map.get_x_size(); x++) {
      for (irt_int y = 0; y < node_map.get_y_size(); y++) {
        std::map<Orientation, GRNode*>& neighbor_ptr_map = node_map[x][y].get_neighbor_ptr_map();
        if (routing_h) {
          if (x != 0) {
            neighbor_ptr_map[Orientation::kWest] = &node_map[x - 1][y];
          }
          if (x != (node_map.get_x_size() - 1)) {
            neighbor_ptr_map[Orientation::kEast] = &node_map[x + 1][y];
          }
        }
        if (routing_v) {
          if (y != 0) {
            neighbor_ptr_map[Orientation::kSouth] = &node_map[x][y - 1];
          }
          if (y != (node_map.get_y_size() - 1)) {
            neighbor_ptr_map[Orientation::kNorth] = &node_map[x][y + 1];
          }
        }
        if (layer_idx != 0) {
          neighbor_ptr_map[Orientation::kDown] = &layer_node_map[layer_idx - 1][x][y];
        }
        if (layer_idx != static_cast<irt_int>(layer_node_map.size()) - 1) {
          neighbor_ptr_map[Orientation::kUp] = &layer_node_map[layer_idx + 1][x][y];
        }
      }
    }
  }
}

void GlobalRouter::buildNodeSupply(GRModel& gr_model)
{
  initNodeRealRect(gr_model);
  addBlockageList(gr_model);
  addNetRegionList(gr_model);
  calcAreaSupply(gr_model);
  buildAccessMap(gr_model);
}

void GlobalRouter::initNodeRealRect(GRModel& gr_model)
{
  GCellAxis& gcell_axis = _gr_data_manager.getDatabase().get_gcell_axis();

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  for (size_t layer_idx = 0; layer_idx < layer_node_map.size(); layer_idx++) {
    GridMap<GRNode>& node_map = layer_node_map[layer_idx];
    for (irt_int x = 0; x < node_map.get_x_size(); x++) {
      for (irt_int y = 0; y < node_map.get_y_size(); y++) {
        GRNode& gr_node = node_map[x][y];
        gr_node.set_real_rect(RTUtil::getRealRect(x, y, gcell_axis));
      }
    }
  }
}

void GlobalRouter::addBlockageList(GRModel& gr_model)
{
  GCellAxis& gcell_axis = _gr_data_manager.getDatabase().get_gcell_axis();
  EXTPlanarRect& die = _gr_data_manager.getDatabase().get_die();
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  std::vector<Blockage>& routing_blockage_list = _gr_data_manager.getDatabase().get_routing_blockage_list();

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();

  for (const Blockage& routing_blockage : routing_blockage_list) {
    irt_int layer_idx = routing_blockage.get_layer_idx();
    irt_int min_spacing = routing_layer_list[layer_idx].getMinSpacing(routing_blockage.get_real_rect());
    PlanarRect enlarged_real_rect = RTUtil::getEnlargedRect(routing_blockage.get_real_rect(), min_spacing, die.get_real_rect());
    PlanarRect enlarged_grid_rect = RTUtil::getClosedGridRect(enlarged_real_rect, gcell_axis);
    for (irt_int x = enlarged_grid_rect.get_lb_x(); x <= enlarged_grid_rect.get_rt_x(); x++) {
      for (irt_int y = enlarged_grid_rect.get_lb_y(); y <= enlarged_grid_rect.get_rt_y(); y++) {
        layer_node_map[layer_idx][x][y].get_net_blockage_map()[-1].push_back(enlarged_real_rect);
      }
    }
  }
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
      for (const EXTLayerRect& routing_shape : gr_pin.get_routing_shape_list()) {
        irt_int layer_idx = routing_shape.get_layer_idx();
        irt_int min_spacing = routing_layer_list[layer_idx].getMinSpacing(routing_shape.get_real_rect());
        PlanarRect enlarged_real_rect = RTUtil::getEnlargedRect(routing_shape.get_real_rect(), min_spacing, die.get_real_rect());
        PlanarRect enlarged_grid_rect = RTUtil::getClosedGridRect(enlarged_real_rect, gcell_axis);
        for (irt_int x = enlarged_grid_rect.get_lb_x(); x <= enlarged_grid_rect.get_rt_x(); x++) {
          for (irt_int y = enlarged_grid_rect.get_lb_y(); y <= enlarged_grid_rect.get_rt_y(); y++) {
            layer_node_map[layer_idx][x][y].get_net_blockage_map()[gr_net.get_net_idx()].push_back(enlarged_real_rect);
          }
        }
      }
    }
  }
}

void GlobalRouter::addNetRegionList(GRModel& gr_model)
{
  GCellAxis& gcell_axis = _gr_data_manager.getDatabase().get_gcell_axis();
  EXTPlanarRect& die = _gr_data_manager.getDatabase().get_die();
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  std::vector<std::vector<ViaMaster>>& layer_via_master_list = _gr_data_manager.getDatabase().get_layer_via_master_list();
  irt_int bottom_routing_layer_idx = _gr_data_manager.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = _gr_data_manager.getConfig().top_routing_layer_idx;

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();

  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    std::vector<EXTLayerRect> net_region_list;
    for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
      for (LayerCoord& real_coord : gr_pin.getRealCoordList()) {
        irt_int layer_idx = real_coord.get_layer_idx();
        for (irt_int via_below_layer_idx : RTUtil::getViaBelowLayerIdxList(layer_idx, bottom_routing_layer_idx, top_routing_layer_idx)) {
          ViaMaster& via_master = layer_via_master_list[via_below_layer_idx].front();

          const LayerRect& below_enclosure = via_master.get_below_enclosure();
          EXTLayerRect below_via_shape;
          below_via_shape.set_real_rect(RTUtil::getOffsetRect(below_enclosure, real_coord));
          below_via_shape.set_layer_idx(below_enclosure.get_layer_idx());
          net_region_list.push_back(below_via_shape);

          const LayerRect& above_enclosure = via_master.get_above_enclosure();
          EXTLayerRect above_via_shape;
          above_via_shape.set_real_rect(RTUtil::getOffsetRect(above_enclosure, real_coord));
          above_via_shape.set_layer_idx(above_enclosure.get_layer_idx());
          net_region_list.push_back(above_via_shape);
        }
      }
    }
    for (const EXTLayerRect& net_region : net_region_list) {
      irt_int layer_idx = net_region.get_layer_idx();
      irt_int min_spacing = routing_layer_list[layer_idx].getMinSpacing(net_region.get_real_rect());
      PlanarRect enlarged_real_rect = RTUtil::getEnlargedRect(net_region.get_real_rect(), min_spacing, die.get_real_rect());
      PlanarRect enlarged_grid_rect = RTUtil::getClosedGridRect(enlarged_real_rect, gcell_axis);
      for (irt_int x = enlarged_grid_rect.get_lb_x(); x <= enlarged_grid_rect.get_rt_x(); x++) {
        for (irt_int y = enlarged_grid_rect.get_lb_y(); y <= enlarged_grid_rect.get_rt_y(); y++) {
          layer_node_map[layer_idx][x][y].get_net_region_map()[gr_net.get_net_idx()].push_back(enlarged_real_rect);
        }
      }
    }
  }
}

void GlobalRouter::calcAreaSupply(GRModel& gr_model)
{
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  // track supply
  for (irt_int layer_idx = 0; layer_idx < static_cast<irt_int>(layer_node_map.size()); layer_idx++) {
    GridMap<GRNode>& node_map = layer_node_map[layer_idx];
#pragma omp parallel for collapse(2)
    for (irt_int x = 0; x < node_map.get_x_size(); x++) {
      for (irt_int y = 0; y < node_map.get_y_size(); y++) {
        initSingleResource(node_map[x][y], routing_layer_list[layer_idx]);
        initResourceSupply(node_map[x][y], routing_layer_list[layer_idx]);
      }
    }
  }
}

void GlobalRouter::initSingleResource(GRNode& gr_node, RoutingLayer& routing_layer)
{
  irt_int min_width = routing_layer.get_min_width();

  double single_wire_area = 0;
  if (routing_layer.isPreferH()) {
    single_wire_area = (gr_node.get_real_rect().getXSpan() * min_width);
  } else {
    single_wire_area = (gr_node.get_real_rect().getYSpan() * min_width);
  }
  gr_node.set_single_wire_area(single_wire_area);

  // 由于通孔是一个一个放的，所以不能算最小面积，在track上放置时，需要以track方向双向延长half spacing
  PlanarRect via_rect(0, 0, routing_layer.get_min_area() / min_width, min_width);
  via_rect.set_rt_x(via_rect.get_rt_x() + routing_layer.getMinSpacing(via_rect));
  gr_node.set_single_via_area(via_rect.getArea());
}

void GlobalRouter::initResourceSupply(GRNode& gr_node, RoutingLayer& routing_layer)
{
  std::map<irt_int, double>& layer_idx_utilization_ratio = _gr_data_manager.getConfig().layer_idx_utilization_ratio;

  double layer_utilization_ratio = 1;
  if (RTUtil::exist(layer_idx_utilization_ratio, routing_layer.get_layer_idx())) {
    layer_utilization_ratio = layer_idx_utilization_ratio[routing_layer.get_layer_idx()];
  }

  std::vector<PlanarRect> wire_list = getWireList(gr_node, routing_layer);

  if (!wire_list.empty()) {
    if (wire_list.front().getArea() != gr_node.get_single_wire_area()) {
      LOG_INST.error(Loc::current(), "The real_wire_area and node_wire_area are not equal!");
    }
  }

  for (auto& [net_idx, blockage_list] : gr_node.get_net_blockage_map()) {
    for (PlanarRect& blockage : blockage_list) {
      std::vector<PlanarRect> new_wire_list;
      for (PlanarRect& wire : wire_list) {
        if (RTUtil::isOpenOverlap(blockage, wire)) {
          // 要切
          if (routing_layer.isPreferH()) {
            if (wire.get_lb_x() < blockage.get_lb_x()) {
              PlanarRect new_wire = wire;
              new_wire.set_rt_x(blockage.get_lb_x());
              new_wire_list.push_back(new_wire);
            }
            if (blockage.get_rt_x() < wire.get_rt_x()) {
              PlanarRect new_wire = wire;
              new_wire.set_lb_x(blockage.get_rt_x());
              new_wire_list.push_back(new_wire);
            }
          } else {
            if (wire.get_lb_y() < blockage.get_lb_y()) {
              PlanarRect new_wire = wire;
              new_wire.set_rt_y(blockage.get_lb_y());
              new_wire_list.push_back(new_wire);
            }
            if (blockage.get_rt_y() < wire.get_rt_y()) {
              PlanarRect new_wire = wire;
              new_wire.set_lb_y(blockage.get_rt_y());
              new_wire_list.push_back(new_wire);
            }
          }
        } else {
          // 不切
          new_wire_list.push_back(wire);
        }
      }
      wire_list = new_wire_list;
    }
  }

  irt_int wire_num = 0;
  irt_int via_num = 0;
  for (PlanarRect& wire : wire_list) {
    if (wire.getArea() == gr_node.get_single_wire_area()) {
      wire_num++;
    } else {
      via_num += static_cast<irt_int>(wire.getArea() / gr_node.get_single_via_area());
    }
  }
  wire_num = static_cast<irt_int>(wire_num * layer_utilization_ratio);
  via_num = static_cast<irt_int>(via_num * layer_utilization_ratio);
  gr_node.set_wire_area_supply(wire_num * gr_node.get_single_wire_area());
  gr_node.set_via_area_supply(via_num * gr_node.get_single_via_area());
}

std::vector<PlanarRect> GlobalRouter::getWireList(GRNode& gr_node, RoutingLayer& routing_layer)
{
  irt_int real_lb_x = gr_node.get_real_rect().get_lb_x();
  irt_int real_lb_y = gr_node.get_real_rect().get_lb_y();
  irt_int real_rt_x = gr_node.get_real_rect().get_rt_x();
  irt_int real_rt_y = gr_node.get_real_rect().get_rt_y();
  std::vector<irt_int> x_list = RTUtil::getOpenScaleList(real_lb_x, real_rt_x, routing_layer.getXTrackGrid());
  std::vector<irt_int> y_list = RTUtil::getOpenScaleList(real_lb_y, real_rt_y, routing_layer.getYTrackGrid());
  irt_int half_width = routing_layer.get_min_width() / 2;

  std::vector<PlanarRect> wire_list;
  if (routing_layer.isPreferH()) {
    for (irt_int y : y_list) {
      wire_list.emplace_back(real_lb_x, y - half_width, real_rt_x, y + half_width);
    }
  } else {
    for (irt_int x : x_list) {
      wire_list.emplace_back(x - half_width, real_lb_y, x + half_width, real_rt_y);
    }
  }
  return wire_list;
}

void GlobalRouter::buildAccessMap(GRModel& gr_model)
{
  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    std::map<LayerCoord, std::vector<LayerCoord>, CmpLayerCoordByXASC> grid_real_list_map;
    for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
      for (AccessPoint& access_point : gr_pin.get_access_point_list()) {
        LayerCoord grid_coord(access_point.get_grid_coord(), access_point.get_layer_idx());
        LayerCoord real_coord(access_point.get_real_coord(), access_point.get_layer_idx());
        grid_real_list_map[grid_coord].push_back(real_coord);
      }
    }
    for (auto& [grid, real_list] : grid_real_list_map) {
      // 本层打通孔
      {
        GRNode& gr_node = layer_node_map[grid.get_layer_idx()][grid.get_x()][grid.get_y()];
        gr_node.get_net_access_map()[gr_net.get_net_idx()].insert({Orientation::kUp, Orientation::kDown});
      }
      // 上层打通孔
      if (grid.get_layer_idx() + 1 < static_cast<irt_int>(layer_node_map.size())) {
        GRNode& gr_node = layer_node_map[grid.get_layer_idx() + 1][grid.get_x()][grid.get_y()];
        gr_node.get_net_access_map()[gr_net.get_net_idx()].insert(
            {Orientation::kUp, Orientation::kDown, Orientation::kEast, Orientation::kWest, Orientation::kSouth, Orientation::kNorth});
      }
      // 下层打通孔
      if (0 <= grid.get_layer_idx() - 1) {
        GRNode& gr_node = layer_node_map[grid.get_layer_idx() - 1][grid.get_x()][grid.get_y()];
        gr_node.get_net_access_map()[gr_net.get_net_idx()].insert(
            {Orientation::kUp, Orientation::kDown, Orientation::kEast, Orientation::kWest, Orientation::kSouth, Orientation::kNorth});
      }
    }
  }
}

void GlobalRouter::buildGRNetPriority(GRModel& gr_model)
{
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    GRNetPriority& gr_net_priority = gr_net.get_gr_net_priority();

    std::vector<GRPin>& gr_pin_list = gr_net.get_gr_pin_list();
    BoundingBox& bounding_box = gr_net.get_bounding_box();

    // connect_type
    gr_net_priority.set_connect_type(gr_net.get_connect_type());
    // routing area
    gr_net_priority.set_routing_area(bounding_box.getTotalSize());
    // length_width_ratio
    double length_width_ratio = bounding_box.getXSize() / 1.0 / bounding_box.getYSize();
    if (length_width_ratio < 1) {
      length_width_ratio = 1 / length_width_ratio;
    }
    gr_net_priority.set_length_width_ratio(length_width_ratio);
    // pin num
    gr_net_priority.set_pin_num(static_cast<irt_int>(gr_pin_list.size()));
  }
}

#endif

#if 1  // check

void GlobalRouter::checkGRModel(GRModel& gr_model)
{
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  irt_int bottom_routing_layer_idx = _gr_data_manager.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = _gr_data_manager.getConfig().top_routing_layer_idx;

  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  for (irt_int layer_idx = 0; layer_idx < static_cast<irt_int>(layer_node_map.size()); layer_idx++) {
    bool routing_h = routing_layer_list[layer_idx].isPreferH();
    bool routing_v = !routing_h;
    if (layer_idx < bottom_routing_layer_idx || top_routing_layer_idx < layer_idx) {
      routing_h = false;
      routing_v = false;
    }
    GridMap<GRNode>& node_map = layer_node_map[layer_idx];
    for (irt_int x = 0; x < node_map.get_x_size(); x++) {
      for (irt_int y = 0; y < node_map.get_y_size(); y++) {
        GRNode& gr_node = node_map[x][y];
        std::map<Orientation, GRNode*>& neighbor_ptr_map = gr_node.get_neighbor_ptr_map();
        if (routing_h) {
          if (RTUtil::exist(neighbor_ptr_map, Orientation::kNorth) || RTUtil::exist(neighbor_ptr_map, Orientation::kSouth)) {
            LOG_INST.error(Loc::current(), "There is illegal vertical neighbor relations!");
          }
        }
        if (routing_v) {
          if (RTUtil::exist(neighbor_ptr_map, Orientation::kEast) || RTUtil::exist(neighbor_ptr_map, Orientation::kWest)) {
            LOG_INST.error(Loc::current(), "There is illegal horizontal neighbor relations!");
          }
        }
        for (auto& [orien, neighbor] : neighbor_ptr_map) {
          Orientation opposite_orien = RTUtil::getOppositeOrientation(orien);
          if (!RTUtil::exist(neighbor->get_neighbor_ptr_map(), opposite_orien)) {
            LOG_INST.error(Loc::current(), "The gr_node neighbor is not bidirection!");
          }
          if (neighbor->get_neighbor_ptr_map()[opposite_orien] != &gr_node) {
            LOG_INST.error(Loc::current(), "The gr_node neighbor is not bidirection!");
          }
          LayerCoord node_coord(gr_node.get_planar_coord(), gr_node.get_layer_idx());
          LayerCoord neighbor_coord(neighbor->get_planar_coord(), neighbor->get_layer_idx());
          if (RTUtil::getOrientation(node_coord, neighbor_coord) != orien) {
            LOG_INST.error(Loc::current(), "The neighbor orien is different with real region!");
          }
        }
        for (auto& [net_idx, blockage_list] : gr_node.get_net_blockage_map()) {
          for (PlanarRect& blockage : blockage_list) {
            if (RTUtil::isClosedOverlap(gr_node.get_real_rect(), blockage)) {
              continue;
            }
            LOG_INST.error(Loc::current(), "The blockage is outside the node region!");
          }
        }
        for (auto& [net_idx, region_list] : gr_node.get_net_region_map()) {
          for (PlanarRect& region : region_list) {
            if (RTUtil::isClosedOverlap(gr_node.get_real_rect(), region)) {
              continue;
            }
            LOG_INST.error(Loc::current(), "The region is outside the node region!");
          }
        }
        if (gr_node.get_single_wire_area() <= 0) {
          LOG_INST.error(Loc::current(), "The single_wire_area <= 0!");
        }
        if (gr_node.get_single_via_area() <= 0) {
          LOG_INST.error(Loc::current(), "The single_via_area <= 0!");
        }
        if (gr_node.get_wire_area_supply() < 0) {
          LOG_INST.error(Loc::current(), "The wire_area_supply < 0!");
        }
        if (gr_node.get_via_area_supply() < 0) {
          LOG_INST.error(Loc::current(), "The via_area_supply < 0!");
        }
      }
    }
  }
}

#endif

#if 1  // sort

void GlobalRouter::sortGRModel(GRModel& gr_model)
{
  Monitor monitor;
  LOG_INST.info(Loc::current(), "Sorting all nets beginning...");

  std::vector<GRNet>& gr_net_list = gr_model.get_gr_net_list();
  std::sort(gr_net_list.begin(), gr_net_list.end(), [&](GRNet& net1, GRNet& net2) { return sortByMultiLevel(net1, net2); });

  LOG_INST.info(Loc::current(), "Sorting all nets completed!", monitor.getStatsInfo());
}

bool GlobalRouter::sortByMultiLevel(GRNet& net1, GRNet& net2)
{
  SortStatus sort_status = SortStatus::kNone;

  sort_status = sortByClockPriority(net1, net2);
  if (sort_status == SortStatus::kTrue) {
    return true;
  } else if (sort_status == SortStatus::kFalse) {
    return false;
  }
  sort_status = sortByRoutingAreaASC(net1, net2);
  if (sort_status == SortStatus::kTrue) {
    return true;
  } else if (sort_status == SortStatus::kFalse) {
    return false;
  }
  sort_status = sortByLengthWidthRatioDESC(net1, net2);
  if (sort_status == SortStatus::kTrue) {
    return true;
  } else if (sort_status == SortStatus::kFalse) {
    return false;
  }
  sort_status = sortByPinNumDESC(net1, net2);
  if (sort_status == SortStatus::kTrue) {
    return true;
  } else if (sort_status == SortStatus::kFalse) {
    return false;
  }
  return false;
}

// 时钟线网优先
SortStatus GlobalRouter::sortByClockPriority(GRNet& net1, GRNet& net2)
{
  ConnectType net1_connect_type = net1.get_gr_net_priority().get_connect_type();
  ConnectType net2_connect_type = net2.get_gr_net_priority().get_connect_type();

  if (net1_connect_type == ConnectType::kClock && net2_connect_type != ConnectType::kClock) {
    return SortStatus::kTrue;
  } else if (net1_connect_type != ConnectType::kClock && net2_connect_type == ConnectType::kClock) {
    return SortStatus::kFalse;
  } else {
    return SortStatus::kEqual;
  }
}

// RoutingArea 升序
SortStatus GlobalRouter::sortByRoutingAreaASC(GRNet& net1, GRNet& net2)
{
  double net1_routing_area = net1.get_gr_net_priority().get_routing_area();
  double net2_routing_area = net2.get_gr_net_priority().get_routing_area();

  if (net1_routing_area < net2_routing_area) {
    return SortStatus::kTrue;
  } else if (net1_routing_area == net2_routing_area) {
    return SortStatus::kEqual;
  } else {
    return SortStatus::kFalse;
  }
}

// 长宽比 降序
SortStatus GlobalRouter::sortByLengthWidthRatioDESC(GRNet& net1, GRNet& net2)
{
  double net1_length_width_ratio = net1.get_gr_net_priority().get_length_width_ratio();
  double net2_length_width_ratio = net2.get_gr_net_priority().get_length_width_ratio();

  if (net1_length_width_ratio > net2_length_width_ratio) {
    return SortStatus::kTrue;
  } else if (net1_length_width_ratio == net2_length_width_ratio) {
    return SortStatus::kEqual;
  } else {
    return SortStatus::kFalse;
  }
}

// PinNum 降序
SortStatus GlobalRouter::sortByPinNumDESC(GRNet& net1, GRNet& net2)
{
  double net1_pin_num = net1.get_gr_net_priority().get_pin_num();
  double net2_pin_num = net2.get_gr_net_priority().get_pin_num();

  if (net1_pin_num > net2_pin_num) {
    return SortStatus::kTrue;
  } else if (net1_pin_num == net2_pin_num) {
    return SortStatus::kEqual;
  } else {
    return SortStatus::kFalse;
  }
}

#endif

#if 1  // route

void GlobalRouter::routeGRModel(GRModel& gr_model)
{
  Monitor monitor;

  std::vector<GRNet>& gr_net_list = gr_model.get_gr_net_list();

  irt_int batch_size = RTUtil::getBatchSize(gr_net_list.size());

  Monitor stage_monitor;
  for (size_t i = 0; i < gr_net_list.size(); i++) {
    routeGRNet(gr_model, gr_net_list[i]);
    if ((i + 1) % batch_size == 0) {
      LOG_INST.info(Loc::current(), "Processed ", (i + 1), " nets", stage_monitor.getStatsInfo());
    }
  }

  LOG_INST.info(Loc::current(), "Processed ", gr_net_list.size(), " nets", monitor.getStatsInfo());
}

void GlobalRouter::routeGRNet(GRModel& gr_model, GRNet& gr_net)
{
  initRoutingInfo(gr_model, gr_net);
  while (!isConnectedAllEnd(gr_model)) {
    routeSinglePath(gr_model);
    rerouteByEnlarging(gr_model);
    rerouteByforcing(gr_model);
    updatePathResult(gr_model);
    resetStartAndEnd(gr_model);
    resetSinglePath(gr_model);
  }
  updateNetResult(gr_model, gr_net);
  resetSingleNet(gr_model);
}

void GlobalRouter::initRoutingInfo(GRModel& gr_model, GRNet& gr_net)
{
  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();
  std::vector<std::vector<GRNode*>>& start_node_comb_list = gr_model.get_start_node_comb_list();
  std::vector<std::vector<GRNode*>>& end_node_comb_list = gr_model.get_end_node_comb_list();

  gr_model.set_wire_unit(1);
  gr_model.set_via_unit(1);
  gr_model.set_gr_net_ref(&gr_net);
  gr_model.set_routing_region(gr_model.get_curr_bounding_box());

  GRPin& gr_driving_pin = gr_net.get_gr_driving_pin();
  std::vector<GRNode*> start_node_comb;
  for (LayerCoord& coord : gr_driving_pin.getGridCoordList()) {
    GRNode* gr_node = &layer_node_map[coord.get_layer_idx()][coord.get_x()][coord.get_y()];
    start_node_comb.push_back(gr_node);
  }
  start_node_comb_list.push_back(start_node_comb);

  for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
    if (gr_pin.get_pin_idx() == gr_driving_pin.get_pin_idx()) {
      continue;
    }
    std::vector<GRNode*> end_node_comb;
    for (LayerCoord& coord : gr_pin.getGridCoordList()) {
      GRNode* gr_node = &layer_node_map[coord.get_layer_idx()][coord.get_x()][coord.get_y()];
      end_node_comb.push_back(gr_node);
    }
    end_node_comb_list.push_back(end_node_comb);
  }
}

bool GlobalRouter::isConnectedAllEnd(GRModel& gr_model)
{
  return gr_model.get_end_node_comb_list().empty();
}

void GlobalRouter::routeSinglePath(GRModel& gr_model)
{
  initPathHead(gr_model);
  while (!searchEnded(gr_model)) {
    expandSearching(gr_model);
    resetPathHead(gr_model);
  }
}

void GlobalRouter::initPathHead(GRModel& gr_model)
{
  std::vector<std::vector<GRNode*>>& start_node_comb_list = gr_model.get_start_node_comb_list();
  std::vector<GRNode*>& path_node_comb = gr_model.get_path_node_comb();

  for (std::vector<GRNode*>& start_node_comb : start_node_comb_list) {
    for (GRNode* start_node : start_node_comb) {
      start_node->set_estimated_cost(getEstimateCostToEnd(gr_model, start_node));
      pushToOpenList(gr_model, start_node);
    }
  }
  for (GRNode* path_node : path_node_comb) {
    path_node->set_estimated_cost(getEstimateCostToEnd(gr_model, path_node));
    pushToOpenList(gr_model, path_node);
  }
  gr_model.set_path_head_node(popFromOpenList(gr_model));
}

bool GlobalRouter::searchEnded(GRModel& gr_model)
{
  std::vector<std::vector<GRNode*>>& end_node_comb_list = gr_model.get_end_node_comb_list();
  GRNode* path_head_node = gr_model.get_path_head_node();

  if (path_head_node == nullptr) {
    gr_model.set_end_node_comb_idx(-1);
    return true;
  }
  for (size_t i = 0; i < end_node_comb_list.size(); i++) {
    for (GRNode* end_node : end_node_comb_list[i]) {
      if (path_head_node == end_node) {
        gr_model.set_end_node_comb_idx(static_cast<irt_int>(i));
        return true;
      }
    }
  }
  return false;
}

void GlobalRouter::expandSearching(GRModel& gr_model)
{
  GRNode* path_head_node = gr_model.get_path_head_node();

  for (auto& [orientation, neighbor_node] : path_head_node->get_neighbor_ptr_map()) {
    if (neighbor_node == nullptr) {
      continue;
    }
    if (!RTUtil::isInside(gr_model.get_routing_region(), *neighbor_node)) {
      continue;
    }
    if (neighbor_node->isClose()) {
      continue;
    }
    if (!passCheckingSegment(gr_model, path_head_node, neighbor_node)) {
      continue;
    }
    if (neighbor_node->isOpen() && replaceParentNode(gr_model, path_head_node, neighbor_node)) {
      neighbor_node->set_known_cost(getKnowCost(gr_model, path_head_node, neighbor_node));
      neighbor_node->set_parent_node(path_head_node);
    } else if (neighbor_node->isNone()) {
      neighbor_node->set_known_cost(getKnowCost(gr_model, path_head_node, neighbor_node));
      neighbor_node->set_parent_node(path_head_node);
      neighbor_node->set_estimated_cost(getEstimateCostToEnd(gr_model, neighbor_node));
      pushToOpenList(gr_model, neighbor_node);
    }
  }
}

bool GlobalRouter::passCheckingSegment(GRModel& gr_model, GRNode* start_node, GRNode* end_node)
{
  if (gr_model.isForcedRouting()) {
    return true;
  }
  Orientation orientation = getOrientation(start_node, end_node);
  if (orientation == Orientation::kNone) {
    return true;
  }
  Orientation opposite_orientation = RTUtil::getOppositeOrientation(orientation);

  GRNode* pre_node = nullptr;
  GRNode* curr_node = start_node;

  while (curr_node != end_node) {
    pre_node = curr_node;
    curr_node = pre_node->getNeighborNode(orientation);

    if (curr_node == nullptr) {
      return false;
    }
    if (pre_node->isOBS(gr_model.get_curr_net_idx(), orientation) || curr_node->isOBS(gr_model.get_curr_net_idx(), opposite_orientation)) {
      return false;
    }
  }
  return true;
}

bool GlobalRouter::replaceParentNode(GRModel& gr_model, GRNode* parent_node, GRNode* child_node)
{
  return getKnowCost(gr_model, parent_node, child_node) < child_node->get_known_cost();
}

void GlobalRouter::resetPathHead(GRModel& gr_model)
{
  gr_model.set_path_head_node(popFromOpenList(gr_model));
}

void GlobalRouter::rerouteByEnlarging(GRModel& gr_model)
{
  Die& die = _gr_data_manager.getDatabase().get_die();
  if (isRoutingFailed(gr_model)) {
    resetSinglePath(gr_model);
    gr_model.set_routing_region(die.get_grid_rect());
    routeSinglePath(gr_model);
    gr_model.set_routing_region(gr_model.get_curr_bounding_box());
    if (!isRoutingFailed(gr_model)) {
      LOG_INST.info(Loc::current(), "The net ", gr_model.get_curr_net_idx(), " enlarged routing successfully!");
    }
  }
}

bool GlobalRouter::isRoutingFailed(GRModel& gr_model)
{
  return gr_model.get_end_node_comb_idx() == -1;
}

void GlobalRouter::resetSinglePath(GRModel& gr_model)
{
  gr_model.set_forced_routing(false);

  std::priority_queue<GRNode*, std::vector<GRNode*>, CmpGRNodeCost> empty_queue;
  gr_model.set_open_queue(empty_queue);

  std::vector<GRNode*>& visited_node_list = gr_model.get_visited_node_list();
  for (GRNode* visited_node : visited_node_list) {
    visited_node->set_state(GRNodeState::kNone);
    visited_node->set_parent_node(nullptr);
    visited_node->set_known_cost(0);
    visited_node->set_estimated_cost(0);
  }
  visited_node_list.clear();

  gr_model.set_path_head_node(nullptr);
  gr_model.set_end_node_comb_idx(-1);
}

void GlobalRouter::rerouteByforcing(GRModel& gr_model)
{
  if (isRoutingFailed(gr_model)) {
    LOG_INST.warning(Loc::current(), "The net ", gr_model.get_curr_net_idx(), " forced routing!");
    resetSinglePath(gr_model);
    gr_model.set_forced_routing(true);
    routeSinglePath(gr_model);
    if (isRoutingFailed(gr_model)) {
      LOG_INST.error(Loc::current(), "The net ", gr_model.get_curr_net_idx(), " forced routing failed!");
    }
  }
}

void GlobalRouter::updatePathResult(GRModel& gr_model)
{
  std::vector<Segment<GRNode*>>& node_segment_list = gr_model.get_node_segment_list();
  GRNode* path_head_node = gr_model.get_path_head_node();

  GRNode* curr_node = path_head_node;
  GRNode* pre_node = curr_node->get_parent_node();

  if (pre_node == nullptr) {
    return;
  }
  Orientation curr_orientation = getOrientation(curr_node, pre_node);
  while (pre_node->get_parent_node() != nullptr) {
    Orientation pre_orientation = getOrientation(pre_node, pre_node->get_parent_node());
    if (curr_orientation != pre_orientation) {
      node_segment_list.emplace_back(curr_node, pre_node);
      curr_orientation = pre_orientation;
      curr_node = pre_node;
    }
    pre_node = pre_node->get_parent_node();
  }
  node_segment_list.emplace_back(curr_node, pre_node);
}

void GlobalRouter::resetStartAndEnd(GRModel& gr_model)
{
  std::vector<std::vector<GRNode*>>& start_node_comb_list = gr_model.get_start_node_comb_list();
  std::vector<std::vector<GRNode*>>& end_node_comb_list = gr_model.get_end_node_comb_list();
  std::vector<GRNode*>& path_node_comb = gr_model.get_path_node_comb();
  GRNode* path_head_node = gr_model.get_path_head_node();
  irt_int end_node_comb_idx = gr_model.get_end_node_comb_idx();

  end_node_comb_list[end_node_comb_idx].clear();
  end_node_comb_list[end_node_comb_idx].push_back(path_head_node);

  GRNode* path_node = path_head_node->get_parent_node();
  if (path_node == nullptr) {
    // 起点和终点重合
    path_node = path_head_node;
  } else {
    // 起点和终点不重合
    while (path_node->get_parent_node() != nullptr) {
      path_node_comb.push_back(path_node);
      path_node = path_node->get_parent_node();
    }
  }
  if (start_node_comb_list.size() == 1) {
    start_node_comb_list.front().clear();
    start_node_comb_list.front().push_back(path_node);
  }
  start_node_comb_list.push_back(end_node_comb_list[end_node_comb_idx]);
  end_node_comb_list.erase(end_node_comb_list.begin() + end_node_comb_idx);
}

void GlobalRouter::updateNetResult(GRModel& gr_model, GRNet& gr_net)
{
  std::vector<Segment<GRNode*>>& node_segment_list = gr_model.get_node_segment_list();

  std::set<GRNode*> wire_set;
  std::set<GRNode*> via_set;

  for (Segment<GRNode*>& node_segment : node_segment_list) {
    GRNode* first_node = node_segment.get_first();
    GRNode* second_node = node_segment.get_second();
    Orientation orientation = getOrientation(first_node, second_node);
    if (orientation == Orientation::kNone || orientation == Orientation::kOblique) {
      LOG_INST.error(Loc::current(), "The orientation is error!");
    }
    GRNode* node_i = first_node;
    while (true) {
      if (orientation == Orientation::kUp || orientation == Orientation::kDown) {
        via_set.insert(node_i);
      } else {
        wire_set.insert(node_i);
      }
      if (node_i == second_node) {
        break;
      }
      node_i = node_i->getNeighborNode(orientation);
    }
  }
  for (GRNode* wire_node : wire_set) {
    wire_node->addWireDemand(gr_model.get_curr_net_idx());
  }
  for (GRNode* via_node : via_set) {
    if (RTUtil::exist(wire_set, via_node)) {
      continue;
    }
    via_node->addViaDemand(gr_model.get_curr_net_idx());
  }

  std::vector<Segment<LayerCoord>>& routing_segment_list = gr_net.get_routing_segment_list();
  for (Segment<GRNode*>& node_segment : node_segment_list) {
    routing_segment_list.emplace_back(*node_segment.get_first(), *node_segment.get_second());
  }
}

void GlobalRouter::resetSingleNet(GRModel& gr_model)
{
  gr_model.set_gr_net_ref(nullptr);
  gr_model.get_start_node_comb_list().clear();
  gr_model.get_end_node_comb_list().clear();
  gr_model.get_path_node_comb().clear();
  gr_model.get_node_segment_list().clear();
}

// manager open list

void GlobalRouter::pushToOpenList(GRModel& gr_model, GRNode* curr_node)
{
  std::priority_queue<GRNode*, std::vector<GRNode*>, CmpGRNodeCost>& open_queue = gr_model.get_open_queue();
  std::vector<GRNode*>& visited_node_list = gr_model.get_visited_node_list();

  open_queue.push(curr_node);
  curr_node->set_state(GRNodeState::kOpen);
  visited_node_list.push_back(curr_node);
}

GRNode* GlobalRouter::popFromOpenList(GRModel& gr_model)
{
  std::priority_queue<GRNode*, std::vector<GRNode*>, CmpGRNodeCost>& open_queue = gr_model.get_open_queue();

  GRNode* gr_node = nullptr;
  if (!open_queue.empty()) {
    gr_node = open_queue.top();
    open_queue.pop();
    gr_node->set_state(GRNodeState::kClose);
  }
  return gr_node;
}

// calculate known cost

double GlobalRouter::getKnowCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node)
{
  double cost = 0;
  cost += start_node->get_known_cost();
  cost += getJointCost(gr_model, end_node, getOrientation(end_node, start_node));
  cost += getWireCost(gr_model, start_node, end_node);
  cost += getViaCost(gr_model, start_node, end_node);
  return cost;
}

double GlobalRouter::getJointCost(GRModel& gr_model, GRNode* curr_node, Orientation orientation)
{
  const PlanarRect& curr_bounding_box = gr_model.get_curr_bounding_box();
  const GridMap<double>& curr_cost_map = gr_model.get_curr_cost_map();

  irt_int local_x = curr_node->get_x() - curr_bounding_box.get_lb_x();
  irt_int local_y = curr_node->get_y() - curr_bounding_box.get_lb_y();
  double net_cost = (curr_cost_map.isInside(local_x, local_y) ? curr_cost_map[local_x][local_y] : 1);

  double env_cost = curr_node->getCost(gr_model.get_curr_net_idx(), orientation);

  double env_weight = 1;
  double net_weight = 1;
  double joint_cost = ((env_weight * env_cost + net_weight * net_cost)
                       * RTUtil::sigmoid((env_weight * env_cost + net_weight * net_cost), (env_weight + net_weight)));
  return joint_cost;
}

// calculate estimate cost

double GlobalRouter::getEstimateCostToEnd(GRModel& gr_model, GRNode* curr_node)
{
  std::vector<std::vector<GRNode*>>& end_node_comb_list = gr_model.get_end_node_comb_list();

  double estimate_cost = DBL_MAX;
  for (std::vector<GRNode*>& end_node_comb : end_node_comb_list) {
    for (GRNode* end_node : end_node_comb) {
      if (end_node->isClose()) {
        continue;
      }
      estimate_cost = std::min(estimate_cost, getEstimateCost(gr_model, curr_node, end_node));
    }
  }
  return estimate_cost;
}

double GlobalRouter::getEstimateCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node)
{
  double estimate_cost = 0;
  estimate_cost += getWireCost(gr_model, start_node, end_node);
  estimate_cost += getViaCost(gr_model, start_node, end_node);
  return estimate_cost;
}

// common

Orientation GlobalRouter::getOrientation(GRNode* start_node, GRNode* end_node)
{
  Orientation orientation = RTUtil::getOrientation(*start_node, *end_node);
  if (orientation == Orientation::kOblique) {
    LOG_INST.error(Loc::current(), "The segment (", (*start_node).get_x(), ",", (*start_node).get_y(), ",", (*start_node).get_layer_idx(),
                   ")-(", (*end_node).get_x(), ",", (*end_node).get_y(), ",", (*end_node).get_layer_idx(), ") is oblique!");
  }
  return orientation;
}

double GlobalRouter::getWireCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node)
{
  return gr_model.get_wire_unit() * RTUtil::getManhattanDistance(*start_node, *end_node);
}

double GlobalRouter::getViaCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node)
{
  return gr_model.get_via_unit() * std::abs(start_node->get_layer_idx() - end_node->get_layer_idx());
}

#endif

#if 1  // plot

void GlobalRouter::plotGRModel(GRModel& gr_model, irt_int curr_net_idx)
{
  GCellAxis& gcell_axis = _gr_data_manager.getDatabase().get_gcell_axis();

  irt_int none_data_type = 0;
  irt_int open_data_type = 10;
  irt_int close_data_type = 20;
  irt_int info_data_type = 30;
  irt_int neighbor_data_type = 40;
  irt_int key_data_type = 50;
  irt_int path_data_type = 60;

  GPGDS gp_gds;

  // node_graph
  GPStruct node_graph_struct("node_graph");
  for (GridMap<GRNode>& node_map : gr_model.get_layer_node_map()) {
    for (irt_int grid_x = 0; grid_x < node_map.get_x_size(); grid_x++) {
      for (irt_int grid_y = 0; grid_y < node_map.get_y_size(); grid_y++) {
        GRNode& gr_node = node_map[grid_x][grid_y];
        PlanarRect real_rect = RTUtil::getRealRect(gr_node.get_planar_coord(), gcell_axis);
        irt_int y_reduced_span = real_rect.getYSpan() / 15;
        irt_int y = real_rect.get_rt_y();

        GPBoundary gp_boundary;
        switch (gr_node.get_state()) {
          case GRNodeState::kNone:
            gp_boundary.set_data_type(none_data_type);
            break;
          case GRNodeState::kOpen:
            gp_boundary.set_data_type(open_data_type);
            break;
          case GRNodeState::kClose:
            gp_boundary.set_data_type(close_data_type);
            break;
          default:
            LOG_INST.error(Loc::current(), "The type is error!");
            break;
        }
        gp_boundary.set_rect(real_rect);
        gp_boundary.set_layer_idx(gr_node.get_layer_idx());
        node_graph_struct.push(gp_boundary);

        y -= y_reduced_span;
        GPText gp_text_node_coord;
        gp_text_node_coord.set_coord(real_rect.get_lb_x(), y);
        gp_text_node_coord.set_text_type(info_data_type);
        gp_text_node_coord.set_message(RTUtil::getString("(", grid_x, " , ", grid_y, " , ", gr_node.get_layer_idx(), ")"));
        gp_text_node_coord.set_layer_idx(gr_node.get_layer_idx());
        gp_text_node_coord.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_node_coord);

        y -= y_reduced_span;
        GPText gp_text_net_blockage_map;
        gp_text_net_blockage_map.set_coord(real_rect.get_lb_x(), y);
        gp_text_net_blockage_map.set_text_type(info_data_type);
        gp_text_net_blockage_map.set_message("net_blockage_map: ");
        gp_text_net_blockage_map.set_layer_idx(gr_node.get_layer_idx());
        gp_text_net_blockage_map.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_net_blockage_map);

        if (!gr_node.get_net_blockage_map().empty()) {
          y -= y_reduced_span;
          GPText gp_text_net_blockage_map_info;
          gp_text_net_blockage_map_info.set_coord(real_rect.get_lb_x(), y);
          gp_text_net_blockage_map_info.set_text_type(info_data_type);
          std::string net_blockage_map_message = "--";
          for (auto& [net_idx, blockage_list] : gr_node.get_net_blockage_map()) {
            net_blockage_map_message += RTUtil::getString("(", net_idx, ")");
          }
          gp_text_net_blockage_map_info.set_message(net_blockage_map_message);
          gp_text_net_blockage_map_info.set_layer_idx(gr_node.get_layer_idx());
          gp_text_net_blockage_map_info.set_presentation(GPTextPresentation::kLeftMiddle);
          node_graph_struct.push(gp_text_net_blockage_map_info);
        }

        y -= y_reduced_span;
        GPText gp_text_net_region_map;
        gp_text_net_region_map.set_coord(real_rect.get_lb_x(), y);
        gp_text_net_region_map.set_text_type(info_data_type);
        gp_text_net_region_map.set_message("net_region_map: ");
        gp_text_net_region_map.set_layer_idx(gr_node.get_layer_idx());
        gp_text_net_region_map.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_net_region_map);

        if (!gr_node.get_net_region_map().empty()) {
          y -= y_reduced_span;
          GPText gp_text_net_region_map_info;
          gp_text_net_region_map_info.set_coord(real_rect.get_lb_x(), y);
          gp_text_net_region_map_info.set_text_type(info_data_type);
          std::string net_region_map_message = "--";
          for (auto& [net_idx, blockage_list] : gr_node.get_net_region_map()) {
            net_region_map_message += RTUtil::getString("(", net_idx, ")");
          }
          gp_text_net_region_map_info.set_message(net_region_map_message);
          gp_text_net_region_map_info.set_layer_idx(gr_node.get_layer_idx());
          gp_text_net_region_map_info.set_presentation(GPTextPresentation::kLeftMiddle);
          node_graph_struct.push(gp_text_net_region_map_info);
        }

        y -= y_reduced_span;
        GPText gp_text_single_wire_area;
        gp_text_single_wire_area.set_coord(real_rect.get_lb_x(), y);
        gp_text_single_wire_area.set_text_type(info_data_type);
        gp_text_single_wire_area.set_message(RTUtil::getString("single_wire_area: ", gr_node.get_single_wire_area()));
        gp_text_single_wire_area.set_layer_idx(gr_node.get_layer_idx());
        gp_text_single_wire_area.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_single_wire_area);

        y -= y_reduced_span;
        GPText gp_text_single_via_area;
        gp_text_single_via_area.set_coord(real_rect.get_lb_x(), y);
        gp_text_single_via_area.set_text_type(info_data_type);
        gp_text_single_via_area.set_message(RTUtil::getString("single_via_area: ", gr_node.get_single_via_area()));
        gp_text_single_via_area.set_layer_idx(gr_node.get_layer_idx());
        gp_text_single_via_area.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_single_via_area);

        y -= y_reduced_span;
        GPText gp_text_wire_area_supply;
        gp_text_wire_area_supply.set_coord(real_rect.get_lb_x(), y);
        gp_text_wire_area_supply.set_text_type(info_data_type);
        gp_text_wire_area_supply.set_message(RTUtil::getString("wire_area_supply: ", gr_node.get_wire_area_supply()));
        gp_text_wire_area_supply.set_layer_idx(gr_node.get_layer_idx());
        gp_text_wire_area_supply.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_wire_area_supply);

        y -= y_reduced_span;
        GPText gp_text_via_area_supply;
        gp_text_via_area_supply.set_coord(real_rect.get_lb_x(), y);
        gp_text_via_area_supply.set_text_type(info_data_type);
        gp_text_via_area_supply.set_message(RTUtil::getString("via_area_supply: ", gr_node.get_via_area_supply()));
        gp_text_via_area_supply.set_layer_idx(gr_node.get_layer_idx());
        gp_text_via_area_supply.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_via_area_supply);

        y -= y_reduced_span;
        GPText gp_text_wire_area_demand;
        gp_text_wire_area_demand.set_coord(real_rect.get_lb_x(), y);
        gp_text_wire_area_demand.set_text_type(info_data_type);
        gp_text_wire_area_demand.set_message(RTUtil::getString("wire_area_demand: ", gr_node.get_wire_area_demand()));
        gp_text_wire_area_demand.set_layer_idx(gr_node.get_layer_idx());
        gp_text_wire_area_demand.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_wire_area_demand);

        y -= y_reduced_span;
        GPText gp_text_via_area_demand;
        gp_text_via_area_demand.set_coord(real_rect.get_lb_x(), y);
        gp_text_via_area_demand.set_text_type(info_data_type);
        gp_text_via_area_demand.set_message(RTUtil::getString("via_area_demand: ", gr_node.get_via_area_demand()));
        gp_text_via_area_demand.set_layer_idx(gr_node.get_layer_idx());
        gp_text_via_area_demand.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_via_area_demand);

        y -= y_reduced_span;
        GPText gp_text_net_queue;
        gp_text_net_queue.set_coord(real_rect.get_lb_x(), y);
        gp_text_net_queue.set_text_type(info_data_type);
        gp_text_net_queue.set_message("net_queue: ");
        gp_text_net_queue.set_layer_idx(gr_node.get_layer_idx());
        gp_text_net_queue.set_presentation(GPTextPresentation::kLeftMiddle);
        node_graph_struct.push(gp_text_net_queue);

        if (!gr_node.get_net_queue().empty()) {
          y -= y_reduced_span;
          GPText gp_text_net_queue_info;
          gp_text_net_queue_info.set_coord(real_rect.get_lb_x(), y);
          gp_text_net_queue_info.set_text_type(info_data_type);
          std::string net_queue_info_message = "--";
          for (irt_int net_idx : RTUtil::getListByQueue(gr_node.get_net_queue())) {
            net_queue_info_message += RTUtil::getString("(", net_idx, ")");
          }
          gp_text_net_queue_info.set_message(net_queue_info_message);
          gp_text_net_queue_info.set_layer_idx(gr_node.get_layer_idx());
          gp_text_net_queue_info.set_presentation(GPTextPresentation::kLeftMiddle);
          node_graph_struct.push(gp_text_net_queue_info);
        }
      }
    }
  }
  gp_gds.addStruct(node_graph_struct);

  // neighbor
  GPStruct neighbor_map_struct("neighbor_map");
  for (GridMap<GRNode>& node_map : gr_model.get_layer_node_map()) {
    for (irt_int grid_x = 0; grid_x < node_map.get_x_size(); grid_x++) {
      for (irt_int grid_y = 0; grid_y < node_map.get_y_size(); grid_y++) {
        GRNode& gr_node = node_map[grid_x][grid_y];
        PlanarRect real_rect = RTUtil::getRealRect(gr_node.get_planar_coord(), gcell_axis);
        irt_int lb_x = real_rect.get_lb_x();
        irt_int lb_y = real_rect.get_lb_y();
        irt_int rt_x = real_rect.get_rt_x();
        irt_int rt_y = real_rect.get_rt_y();
        irt_int mid_x = (lb_x + rt_x) / 2;
        irt_int mid_y = (lb_y + rt_y) / 2;
        irt_int x_reduced_span = (rt_x - lb_x) / 4;
        irt_int y_reduced_span = (rt_y - lb_y) / 4;
        irt_int width = std::min(x_reduced_span, y_reduced_span) / 2;

        for (auto& [orientation, neighbor_node] : gr_node.get_neighbor_ptr_map()) {
          GPPath gp_path;
          switch (orientation) {
            case Orientation::kEast:
              gp_path.set_segment(rt_x - x_reduced_span, mid_y, rt_x, mid_y);
              break;
            case Orientation::kSouth:
              gp_path.set_segment(mid_x, lb_y, mid_x, lb_y + y_reduced_span);
              break;
            case Orientation::kWest:
              gp_path.set_segment(lb_x, mid_y, lb_x + x_reduced_span, mid_y);
              break;
            case Orientation::kNorth:
              gp_path.set_segment(mid_x, rt_y - y_reduced_span, mid_x, rt_y);
              break;
            case Orientation::kUp:
              gp_path.set_segment(rt_x - x_reduced_span, rt_y - y_reduced_span, rt_x, rt_y);
              break;
            case Orientation::kDown:
              gp_path.set_segment(lb_x, lb_y, lb_x + x_reduced_span, lb_y + y_reduced_span);
              break;
            default:
              LOG_INST.error(Loc::current(), "The orientation is oblique!");
              break;
          }
          gp_path.set_layer_idx(gr_node.get_layer_idx());
          gp_path.set_width(width);
          gp_path.set_data_type(neighbor_data_type);
          neighbor_map_struct.push(gp_path);
        }
      }
    }
  }
  gp_gds.addStruct(neighbor_map_struct);

  // net
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    GPStruct net_struct(RTUtil::getString("net_", gr_net.get_net_idx()));

    if (curr_net_idx == -1 || gr_net.get_net_idx() == curr_net_idx) {
      for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
        for (LayerCoord& coord : gr_pin.getGridCoordList()) {
          PlanarRect real_rect = RTUtil::getRealRect(coord.get_planar_coord(), gcell_axis);

          GPBoundary gp_boundary;
          gp_boundary.set_data_type(key_data_type);
          gp_boundary.set_rect(real_rect);
          gp_boundary.set_layer_idx(coord.get_layer_idx());
          net_struct.push(gp_boundary);
        }
      }
    }
    for (Segment<LayerCoord>& segment : gr_net.get_routing_segment_list()) {
      LayerCoord first_coord = segment.get_first();
      irt_int first_layer_idx = first_coord.get_layer_idx();
      LayerCoord second_coord = segment.get_second();
      irt_int second_layer_idx = second_coord.get_layer_idx();

      if (first_layer_idx == second_layer_idx) {
        GPBoundary gp_boundary;
        gp_boundary.set_data_type(path_data_type);
        gp_boundary.set_rect(RTUtil::getRealRect(first_coord, second_coord, gcell_axis));
        gp_boundary.set_layer_idx(first_layer_idx);
        net_struct.push(gp_boundary);
      } else {
        RTUtil::sortASC(first_layer_idx, second_layer_idx);
        for (irt_int layer_idx = first_layer_idx; layer_idx <= second_layer_idx; layer_idx++) {
          GPBoundary gp_boundary;
          gp_boundary.set_data_type(path_data_type);
          gp_boundary.set_rect(RTUtil::getRealRect(first_coord, gcell_axis));
          gp_boundary.set_layer_idx(layer_idx);
          net_struct.push(gp_boundary);
        }
      }
    }
    gp_gds.addStruct(net_struct);
  }
  GP_INST.plot(gp_gds, _gr_data_manager.getConfig().temp_directory_path + "gr_model.gds", false, false);
}

#endif

#if 1  // update

void GlobalRouter::updateGRModel(GRModel& gr_model)
{
#pragma omp parallel for
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    initRoutingResult(gr_net);
    buildRoutingResult(gr_net);
  }
  updateOriginGRResultTree(gr_model);
}

void GlobalRouter::initRoutingResult(GRNet& gr_net)
{
  std::vector<LayerCoord> driving_grid_coord_list = gr_net.get_gr_driving_pin().getGridCoordList();
  std::vector<Segment<LayerCoord>>& routing_segment_list = gr_net.get_routing_segment_list();
  std::map<LayerCoord, std::set<irt_int>, CmpLayerCoordByXASC> key_coord_pin_map;
  for (GRPin& gr_pin : gr_net.get_gr_pin_list()) {
    for (LayerCoord& grid_coord : gr_pin.getGridCoordList()) {
      key_coord_pin_map[grid_coord].insert(gr_pin.get_pin_idx());
    }
  }
  MTree<LayerCoord> coord_tree = RTUtil::getTreeByFullFlow(driving_grid_coord_list, routing_segment_list, key_coord_pin_map);
  std::function<RTNode(LayerCoord&, std::map<LayerCoord, std::set<irt_int>, CmpLayerCoordByXASC>&)> convert
      = std::bind(&GlobalRouter::convertToRTNode, this, std::placeholders::_1, std::placeholders::_2);
  gr_net.set_gr_result_tree(RTUtil::convertTree(coord_tree, convert, key_coord_pin_map));
}

RTNode GlobalRouter::convertToRTNode(LayerCoord& coord, std::map<LayerCoord, std::set<irt_int>, CmpLayerCoordByXASC>& key_coord_pin_map)
{
  GCellAxis& gcell_axis = _gr_data_manager.getDatabase().get_gcell_axis();

  Guide guide(RTUtil::getRealRect(coord, gcell_axis), coord.get_layer_idx(), coord.get_planar_coord());

  RTNode rt_node;
  rt_node.set_first_guide(guide);
  rt_node.set_second_guide(guide);
  if (RTUtil::exist(key_coord_pin_map, coord)) {
    rt_node.set_pin_idx_set(key_coord_pin_map[coord]);
  }
  return rt_node;
}

void GlobalRouter::buildRoutingResult(GRNet& gr_net)
{
  if (gr_net.get_gr_result_tree().get_root() == nullptr) {
    return;
  }
  std::map<TNode<RTNode>*, TNode<RTNode>*> origin_to_merged_map;
  std::queue<TNode<RTNode>*> node_queue = RTUtil::initQueue(gr_net.get_gr_result_tree().get_root());
  while (!node_queue.empty()) {
    TNode<RTNode>* curr_node = RTUtil::getFrontAndPop(node_queue);
    RTUtil::addListToQueue(node_queue, curr_node->get_child_list());

    std::vector<TNode<RTNode>*> box_node_list;
    std::vector<TNode<RTNode>*> bridge_node_list;
    for (TNode<RTNode>* child_node : curr_node->get_child_list()) {
      PlanarCoord& curr_grid_coord = curr_node->value().get_first_guide().get_grid_coord();
      PlanarCoord& child_grid_coord = child_node->value().get_first_guide().get_grid_coord();
      if (curr_grid_coord == child_grid_coord) {
        box_node_list.push_back(child_node);
      } else {
        bridge_node_list.push_back(child_node);
      }
    }
    TNode<RTNode>* merged_node = curr_node;
    if (RTUtil::exist(origin_to_merged_map, curr_node)) {
      merged_node = origin_to_merged_map[curr_node];
    }
    for (TNode<RTNode>* child_node : box_node_list) {
      buildDRNode(merged_node, child_node);
      origin_to_merged_map[child_node] = merged_node;
    }
    for (TNode<RTNode>* child_node : bridge_node_list) {
      buildTANode(merged_node, child_node);
    }
  }
}

void GlobalRouter::buildDRNode(TNode<RTNode>* parent_node, TNode<RTNode>* child_node)
{
  irt_int child_layer_idx = child_node->value().get_first_guide().get_layer_idx();
  Guide& first_guide = parent_node->value().get_first_guide();
  Guide& second_guide = parent_node->value().get_second_guide();
  first_guide.set_layer_idx(std::min(first_guide.get_layer_idx(), child_layer_idx));
  second_guide.set_layer_idx(std::max(second_guide.get_layer_idx(), child_layer_idx));

  std::set<int>& parent_pin_idx_set = parent_node->value().get_pin_idx_set();
  std::set<int>& child_pin_idx_set = child_node->value().get_pin_idx_set();
  parent_pin_idx_set.insert(child_pin_idx_set.begin(), child_pin_idx_set.end());

  parent_node->addChildren(child_node->get_child_list());
  parent_node->delChild(child_node);
}

void GlobalRouter::buildTANode(TNode<RTNode>* parent_node, TNode<RTNode>* child_node)
{
  Guide first_guide = parent_node->value().get_first_guide();
  Guide second_guide = child_node->value().get_first_guide();
  first_guide.set_layer_idx(second_guide.get_layer_idx());
  if (!CmpPlanarCoordByXASC()(first_guide.get_grid_coord(), second_guide.get_grid_coord())) {
    std::swap(first_guide, second_guide);
  }
  RTNode rt_node;
  rt_node.set_first_guide(first_guide);
  rt_node.set_second_guide(second_guide);

  TNode<RTNode>* bridge_node = new TNode<RTNode>(rt_node);
  parent_node->addChild(bridge_node);
  bridge_node->addChild(child_node);
  parent_node->delChild(child_node);
}

void GlobalRouter::updateOriginGRResultTree(GRModel& gr_model)
{
  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    Net* origin_net = gr_net.get_origin_net();
    origin_net->set_gr_result_tree(gr_net.get_gr_result_tree());
  }
}

#endif

#if 1  // report

void GlobalRouter::reportGRModel(GRModel& gr_model)
{
  countGRModel(gr_model);
  reportTable(gr_model);
}

void GlobalRouter::countGRModel(GRModel& gr_model)
{
  irt_int micron_dbu = _gr_data_manager.getDatabase().get_micron_dbu();
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  std::vector<std::vector<ViaMaster>>& layer_via_master_list = _gr_data_manager.getDatabase().get_layer_via_master_list();

  GRModelStat& gr_model_stat = gr_model.get_gr_model_stat();

  for (GRNet& gr_net : gr_model.get_gr_net_list()) {
    for (TNode<RTNode>* node : RTUtil::getNodeList(gr_net.get_gr_result_tree())) {
      Guide& first_guide = node->value().get_first_guide();
      irt_int first_layer_idx = first_guide.get_layer_idx();
      Guide& second_guide = node->value().get_second_guide();
      irt_int second_layer_idx = second_guide.get_layer_idx();

      if (first_layer_idx == second_layer_idx) {
        double wire_length = RTUtil::getManhattanDistance(first_guide.getMidPoint(), second_guide.getMidPoint()) / 1.0 / micron_dbu;
        gr_model_stat.addTotalWireLength(wire_length);
        gr_model_stat.get_routing_wire_length_map()[first_layer_idx] += wire_length;
      } else {
        gr_model_stat.addTotalViaNumber(std::abs(first_layer_idx - second_layer_idx));
        for (irt_int i = std::min(first_layer_idx, second_layer_idx); i < std::max(first_layer_idx, second_layer_idx); i++) {
          gr_model_stat.get_cut_via_number_map()[layer_via_master_list[i].front().get_cut_layer_idx()]++;
        }
      }
    }
  }
  std::vector<GridMap<GRNode>>& layer_node_map = gr_model.get_layer_node_map();

  double max_wire_overflow = DBL_MIN;
  double max_via_overflow = DBL_MIN;
  for (RoutingLayer& routing_layer : routing_layer_list) {
    GridMap<GRNode>& node_map = layer_node_map[routing_layer.get_layer_idx()];
    for (irt_int grid_x = 0; grid_x < node_map.get_x_size(); grid_x++) {
      for (irt_int grid_y = 0; grid_y < node_map.get_y_size(); grid_y++) {
        GRNode& gr_node = node_map[grid_x][grid_y];
        double wire_remain = gr_node.get_wire_area_supply() + std::min(gr_node.get_via_area_supply() - gr_node.get_via_area_demand(), 0)
                             - gr_node.get_wire_area_demand();
        double wire_overflow = wire_remain / gr_node.get_single_wire_area() * (-1);
        max_wire_overflow = std::max(max_wire_overflow, wire_overflow);
        gr_model_stat.get_wire_overflow_list().push_back(wire_overflow);

        double via_remain = gr_node.get_wire_area_supply() - gr_node.get_wire_area_demand() + gr_node.get_via_area_supply()
                            - gr_node.get_via_area_demand();
        double via_overflow = via_remain / gr_node.get_single_via_area() * (-1);
        max_via_overflow = std::max(max_via_overflow, via_overflow);
        gr_model_stat.get_via_overflow_list().push_back(via_overflow);
      }
    }
  }
  gr_model_stat.set_max_wire_overflow(max_wire_overflow);
  gr_model_stat.set_max_via_overflow(max_via_overflow);
}

void GlobalRouter::reportTable(GRModel& gr_model)
{
  std::vector<RoutingLayer>& routing_layer_list = _gr_data_manager.getDatabase().get_routing_layer_list();
  std::vector<CutLayer>& cut_layer_list = _gr_data_manager.getDatabase().get_cut_layer_list();

  GRModelStat& gr_model_stat = gr_model.get_gr_model_stat();

  // report wire info
  fort::char_table wire_table;
  wire_table.set_border_style(FT_SOLID_STYLE);
  wire_table << fort::header << "Routing Layer"
             << "Wire Length / um" << fort::endr;
  for (RoutingLayer& routing_layer : routing_layer_list) {
    double layer_wire_length = gr_model_stat.get_routing_wire_length_map()[routing_layer.get_layer_idx()];
    wire_table << routing_layer.get_layer_name()
               << RTUtil::getString(layer_wire_length, "(", RTUtil::getPercentage(layer_wire_length, gr_model_stat.get_total_wire_length()),
                                    "%)")
               << fort::endr;
  }
  wire_table << fort::header << "Total" << gr_model_stat.get_total_wire_length() << fort::endr;
  for (std::string table_str : RTUtil::splitString(wire_table.to_string(), '\n')) {
    LOG_INST.info(Loc::current(), table_str);
  }

  // report via info
  fort::char_table via_table;
  via_table.set_border_style(FT_SOLID_STYLE);
  via_table << fort::header << "Cut Layer"
            << "Via Number" << fort::endr;
  for (CutLayer& cut_layer : cut_layer_list) {
    irt_int layer_via_number = gr_model_stat.get_cut_via_number_map()[cut_layer.get_layer_idx()];
    via_table << cut_layer.get_layer_name()
              << RTUtil::getString(layer_via_number, "(", RTUtil::getPercentage(layer_via_number, gr_model_stat.get_total_via_number()),
                                   "%)")
              << fort::endr;
  }
  via_table << fort::header << "Total" << gr_model_stat.get_total_via_number() << fort::endr;
  for (std::string table_str : RTUtil::splitString(via_table.to_string(), '\n')) {
    LOG_INST.info(Loc::current(), table_str);
  }

  // report wire overflow info
  std::vector<double>& wire_overflow_list = gr_model_stat.get_wire_overflow_list();
  double wire_overflow_range = RTUtil::getScaleRange(wire_overflow_list);
  GridMap<double> wire_overflow_map = RTUtil::getRangeNumRatioMap(wire_overflow_list);

  fort::char_table wire_overflow_table;
  wire_overflow_table.set_border_style(FT_SOLID_STYLE);
  wire_overflow_table << fort::header << "Wire Overflow"
                      << "GCell Number" << fort::endr;
  for (irt_int y_idx = 0; y_idx < wire_overflow_map.get_y_size(); y_idx++) {
    double left = wire_overflow_map[0][y_idx];
    double right = left + wire_overflow_range;
    std::string range_str;
    if (right >= gr_model_stat.get_max_wire_overflow()) {
      range_str = RTUtil::getString("[", left, ",", gr_model_stat.get_max_wire_overflow(), "]");
    } else {
      range_str = RTUtil::getString("[", left, ",", right, ")");
    }
    wire_overflow_table << range_str << RTUtil::getString(wire_overflow_map[1][y_idx], "(", wire_overflow_map[2][y_idx], "%)")
                        << fort::endr;
  }
  wire_overflow_table << fort::header << "Total" << wire_overflow_list.size() << fort::endr;
  for (std::string table_str : RTUtil::splitString(wire_overflow_table.to_string(), '\n')) {
    LOG_INST.info(Loc::current(), table_str);
  }

  // report via overflow info
  std::vector<double>& via_overflow_list = gr_model_stat.get_via_overflow_list();
  double via_overflow_range = RTUtil::getScaleRange(via_overflow_list);
  GridMap<double> via_overflow_map = RTUtil::getRangeNumRatioMap(via_overflow_list);

  fort::char_table via_overflow_table;
  via_overflow_table.set_border_style(FT_SOLID_STYLE);
  via_overflow_table << fort::header << "Via Overflow"
                     << "GCell Number" << fort::endr;
  for (irt_int y_idx = 0; y_idx < via_overflow_map.get_y_size(); y_idx++) {
    double left = via_overflow_map[0][y_idx];
    double right = left + via_overflow_range;
    std::string range_str;
    if (right >= gr_model_stat.get_max_via_overflow()) {
      range_str = RTUtil::getString("[", left, ",", gr_model_stat.get_max_via_overflow(), "]");
    } else {
      range_str = RTUtil::getString("[", left, ",", right, ")");
    }
    via_overflow_table << range_str << RTUtil::getString(via_overflow_map[1][y_idx], "(", via_overflow_map[2][y_idx], "%)") << fort::endr;
  }
  via_overflow_table << fort::header << "Total" << via_overflow_list.size() << fort::endr;
  for (std::string table_str : RTUtil::splitString(via_overflow_table.to_string(), '\n')) {
    LOG_INST.info(Loc::current(), table_str);
  }
}

#endif

}  // namespace irt
