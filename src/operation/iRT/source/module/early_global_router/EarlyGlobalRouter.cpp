#include "EarlyGlobalRouter.hpp"

using namespace std;
namespace irt {
// public

void EarlyGlobalRouter::initInst(std::map<std::string, std::any>& config_map, idb::IdbBuilder* idb_builder)
{
  if (_egr_instance == nullptr) {
    _egr_instance = new EarlyGlobalRouter(config_map, idb_builder);
  }
}

EarlyGlobalRouter& EarlyGlobalRouter::getInst()
{
  if (_egr_instance == nullptr) {
    LOG_INST.error(Loc::current(), "The instance not initialized!");
  }
  return *_egr_instance;
}

void EarlyGlobalRouter::destroyInst()
{
  if (_egr_instance != nullptr) {
    delete _egr_instance;
    _egr_instance = nullptr;
  }
}

void EarlyGlobalRouter::route()
{
  Monitor monitor;

  std::vector<EGRNet>& egr_net_list = _egr_data_manager.getDatabase().get_egr_net_list();
  routeEGRNetList(egr_net_list);
  reportEGRNetList();
  LOG_INST.info(Loc::current(), "The early_global_router completed!", monitor.getStatsInfo());

  // LOG_INST.destroyInst();
  // recordLog(_egr_data_manager.getConfig().temp_directory_path + "egr_record.log");
}

void EarlyGlobalRouter::recordLog(std::string record_file_path)
{
  std::ofstream record_file_stream = std::ofstream(record_file_path, std::ios_base::app);

  std::ifstream log_file_stream = std::ifstream(_egr_data_manager.getConfig().log_file_path);

  std::string split_string
      = "##################################################################################################################################"
        "##################################################################";
  std::vector<std::string> record_list = {"printConfig",         "printDatabase",
                                          "reportCongestion",    "reportWireViaStatistics",
                                          "early_global_router", "reportCongestion Info] Remain",
                                          "Info] Processed"};

  record_file_stream << split_string << std::endl;
  record_file_stream << "update time: " << RTUtil::getTimestamp() << std::endl;

  std::string new_line;
  while (getline(log_file_stream, new_line)) {
    bool is_record = false;
    for (std::string record : record_list) {
      if (std::string::npos == new_line.find(record)) {
        continue;
      }
      is_record = true;
      break;
    }
    if (is_record) {
      record_file_stream << new_line << std::endl;
    }
  }
  record_file_stream << split_string << std::endl;
}

void EarlyGlobalRouter::plot()
{
  irt_int node_data_type = 0;
  irt_int blockage_data_type = 1;
  irt_int text_data_type = 2;

  std::vector<Blockage>& routing_blockage_list = _egr_data_manager.getDatabase().get_routing_blockage_list();
  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();

  std::ofstream* gds_file = RTUtil::getOutputFileStream(_egr_data_manager.getConfig().temp_directory_path + "egr.gds");

  RTUtil::pushStream(gds_file, "HEADER 600", "\n");
  RTUtil::pushStream(gds_file, "BGNLIB", "\n");
  RTUtil::pushStream(gds_file, "LIBNAME early_global_router", "\n");
  RTUtil::pushStream(gds_file, "UNITS 0.001 1e-9", "\n");

  // routing_blockage_list
  RTUtil::pushStream(gds_file, "BGNSTR", "\n");
  RTUtil::pushStream(gds_file, "STRNAME ", "routing_blockage_list", "\n");
  for (Blockage& routing_blockage : routing_blockage_list) {
    irt_int lb_x = routing_blockage.get_real_lb_x();
    irt_int lb_y = routing_blockage.get_real_lb_y();
    irt_int rt_x = routing_blockage.get_real_rt_x();
    irt_int rt_y = routing_blockage.get_real_rt_y();

    RTUtil::pushStream(gds_file, "BOUNDARY", "\n");
    RTUtil::pushStream(gds_file, "LAYER ", routing_blockage.get_layer_idx(), "\n");
    RTUtil::pushStream(gds_file, "DATATYPE ", blockage_data_type, "\n");
    RTUtil::pushStream(gds_file, "XY", "\n");
    RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
    RTUtil::pushStream(gds_file, rt_x, " : ", lb_y, "\n");
    RTUtil::pushStream(gds_file, rt_x, " : ", rt_y, "\n");
    RTUtil::pushStream(gds_file, lb_x, " : ", rt_y, "\n");
    RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
    RTUtil::pushStream(gds_file, "ENDEL", "\n");
  }
  RTUtil::pushStream(gds_file, "ENDSTR", "\n");

  // layer_resource_map
  RTUtil::pushStream(gds_file, "BGNSTR", "\n");
  RTUtil::pushStream(gds_file, "STRNAME ", "layer_resource_map", "\n");
  for (size_t layer_idx = 0; layer_idx < layer_resource_map.size(); layer_idx++) {
    GridMap<EGRNode>& resource_map = layer_resource_map[layer_idx];
    for (irt_int x = 0; x < resource_map.get_x_size(); x++) {
      for (irt_int y = 0; y < resource_map.get_y_size(); y++) {
        EGRNode& node = resource_map[x][y];
        irt_int lb_x = node.get_lb_x();
        irt_int lb_y = node.get_lb_y();
        irt_int rt_x = node.get_rt_x();
        irt_int rt_y = node.get_rt_y();

        RTUtil::pushStream(gds_file, "BOUNDARY", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "DATATYPE ", node_data_type, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
        RTUtil::pushStream(gds_file, rt_x, " : ", lb_y, "\n");
        RTUtil::pushStream(gds_file, rt_x, " : ", rt_y, "\n");
        RTUtil::pushStream(gds_file, lb_x, " : ", rt_y, "\n");
        RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");

        irt_int mid_x = (lb_x + rt_x) / 2;
        irt_int mid_y = (lb_y + rt_y) / 2;

        RTUtil::pushStream(gds_file, "TEXT", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "TEXTTYPE ", text_data_type, "\n");
        RTUtil::pushStream(gds_file, "PRESENTATION ", 4, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, lb_x, " : ", mid_y, "\n");
        RTUtil::pushStream(gds_file, "STRING ", RTUtil::getString("west: ", node.get_west_demand(), "/", node.get_west_supply()), "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");

        RTUtil::pushStream(gds_file, "TEXT", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "TEXTTYPE ", text_data_type, "\n");
        RTUtil::pushStream(gds_file, "PRESENTATION ", 6, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, rt_x, " : ", mid_y, "\n");
        RTUtil::pushStream(gds_file, "STRING ", RTUtil::getString("east: ", node.get_east_demand(), "/", node.get_east_supply()), "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");

        RTUtil::pushStream(gds_file, "TEXT", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "TEXTTYPE ", text_data_type, "\n");
        RTUtil::pushStream(gds_file, "PRESENTATION ", 5, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, mid_x, " : ", mid_y, "\n");
        RTUtil::pushStream(gds_file, "STRING ", RTUtil::getString("track: ", node.get_track_demand(), "/", node.get_track_supply()), "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");

        RTUtil::pushStream(gds_file, "TEXT", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "TEXTTYPE ", text_data_type, "\n");
        RTUtil::pushStream(gds_file, "PRESENTATION ", 9, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, mid_x, " : ", lb_y, "\n");
        RTUtil::pushStream(gds_file, "STRING ", RTUtil::getString("south: ", node.get_south_demand(), "/", node.get_south_supply()), "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");

        RTUtil::pushStream(gds_file, "TEXT", "\n");
        RTUtil::pushStream(gds_file, "LAYER ", layer_idx, "\n");
        RTUtil::pushStream(gds_file, "TEXTTYPE ", text_data_type, "\n");
        RTUtil::pushStream(gds_file, "PRESENTATION ", 1, "\n");
        RTUtil::pushStream(gds_file, "XY", "\n");
        RTUtil::pushStream(gds_file, mid_x, " : ", rt_y, "\n");
        RTUtil::pushStream(gds_file, "STRING ", RTUtil::getString("north: ", node.get_north_demand(), "/", node.get_north_supply()), "\n");
        RTUtil::pushStream(gds_file, "ENDEL", "\n");
      }
    }
  }
  RTUtil::pushStream(gds_file, "ENDSTR", "\n");

  // top
  RTUtil::pushStream(gds_file, "BGNSTR", "\n");
  RTUtil::pushStream(gds_file, "STRNAME ", "top", "\n");
  RTUtil::pushStream(gds_file, "SREF", "\n");
  RTUtil::pushStream(gds_file, "SNAME routing_blockage_list\n");
  RTUtil::pushStream(gds_file, "XY 0:0", "\n");
  RTUtil::pushStream(gds_file, "ENDEL", "\n");
  RTUtil::pushStream(gds_file, "SREF", "\n");
  RTUtil::pushStream(gds_file, "SNAME layer_resource_map\n");
  RTUtil::pushStream(gds_file, "XY 0:0", "\n");
  RTUtil::pushStream(gds_file, "ENDEL", "\n");
  RTUtil::pushStream(gds_file, "ENDSTR", "\n");

  RTUtil::pushStream(gds_file, "ENDLIB", "\n");
  RTUtil::closeFileStream(gds_file);
}

void EarlyGlobalRouter::plotCongstLoc()
{
  const int kLevel = 9;
  irt_int node_data_type = 0;
  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();
  std::ofstream* gds_file = RTUtil::getOutputFileStream(_egr_data_manager.getConfig().temp_directory_path + "egr_congst_loc.gds");

  RTUtil::pushStream(gds_file, "HEADER 600", "\n");
  RTUtil::pushStream(gds_file, "BGNLIB", "\n");
  RTUtil::pushStream(gds_file, "LIBNAME early_global_router", "\n");
  RTUtil::pushStream(gds_file, "UNITS 0.001 1e-9", "\n");

  // congstion_map
  irt_int x_size = layer_resource_map[0].get_x_size();
  irt_int y_size = layer_resource_map[0].get_y_size();

  std::vector<std::vector<irt_int>> cong_map(x_size, std::vector<int>(y_size, 0));
  irt_int max_congestion_val = 0;
  for (size_t layer_idx = 0; layer_idx < layer_resource_map.size(); layer_idx++) {
    GridMap<EGRNode>& resource_map = layer_resource_map[layer_idx];
    for (irt_int x = 0; x < x_size; x++) {
      for (irt_int y = 0; y < y_size; y++) {
        EGRNode& node = resource_map[x][y];
        double congestion_val = 0;
        for (EGRResourceType resource_type :
             {EGRResourceType::kNorth, EGRResourceType::kSouth, EGRResourceType::kWest, EGRResourceType::kEast}) {
          if (node.getSupply(resource_type) < node.getDemand(resource_type)) {
            congestion_val += (node.getDemand(resource_type) - node.getSupply(resource_type));
          }
          cong_map[x][y] += static_cast<irt_int>(std::ceil(congestion_val / 2));
          max_congestion_val = max_congestion_val > cong_map[x][y] ? max_congestion_val : cong_map[x][y];
        }
      }
    }
  }
  irt_int interval = max(max_congestion_val / kLevel, 1);
  RTUtil::pushStream(gds_file, "BGNSTR", "\n");
  RTUtil::pushStream(gds_file, "STRNAME ", "congstion_map", "\n");

  GridMap<EGRNode>& resource_map = layer_resource_map[0];
  for (irt_int x = 0; x < x_size; x++) {
    for (irt_int y = 0; y < y_size; y++) {
      EGRNode& node = resource_map[x][y];
      irt_int lb_x = node.get_lb_x();
      irt_int lb_y = node.get_lb_y();
      irt_int rt_x = node.get_rt_x();
      irt_int rt_y = node.get_rt_y();
      RTUtil::pushStream(gds_file, "BOUNDARY", "\n");
      RTUtil::pushStream(gds_file, "LAYER ", static_cast<irt_int>(std::ceil(cong_map[x][y] * 1.0 / interval)), "\n");
      RTUtil::pushStream(gds_file, "DATATYPE ", node_data_type, "\n");
      RTUtil::pushStream(gds_file, "XY", "\n");
      RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
      RTUtil::pushStream(gds_file, rt_x, " : ", lb_y, "\n");
      RTUtil::pushStream(gds_file, rt_x, " : ", rt_y, "\n");
      RTUtil::pushStream(gds_file, lb_x, " : ", rt_y, "\n");
      RTUtil::pushStream(gds_file, lb_x, " : ", lb_y, "\n");
      RTUtil::pushStream(gds_file, "ENDEL", "\n");
    }
  }
  RTUtil::pushStream(gds_file, "ENDSTR", "\n");

  // top
  RTUtil::pushStream(gds_file, "BGNSTR", "\n");
  RTUtil::pushStream(gds_file, "STRNAME ", "top", "\n");
  RTUtil::pushStream(gds_file, "SREF", "\n");
  RTUtil::pushStream(gds_file, "SNAME congstion_map\n");
  RTUtil::pushStream(gds_file, "XY 0:0", "\n");
  RTUtil::pushStream(gds_file, "ENDEL", "\n");
  RTUtil::pushStream(gds_file, "ENDSTR", "\n");

  RTUtil::pushStream(gds_file, "ENDLIB", "\n");
  RTUtil::closeFileStream(gds_file);
}

// private

EarlyGlobalRouter* EarlyGlobalRouter::_egr_instance = nullptr;

void EarlyGlobalRouter::init(std::map<std::string, std::any>& config_map, idb::IdbBuilder* idb_builder)
{
  _idb_builder = idb_builder;
  Logger::initInst();
  _egr_data_manager.input(config_map, idb_builder);
}

void EarlyGlobalRouter::destroy()
{
  LOG_INST.destroyInst();
}

void EarlyGlobalRouter::routeEGRNetList(std::vector<EGRNet>& egr_net_list)
{
  Monitor monitor;

  irt_int batch_size = RTUtil::getBatchSize(egr_net_list.size());

  Monitor stage_monitor;
  for (size_t i = 0; i < egr_net_list.size(); i++) {
    routeEGRNet(egr_net_list[i]);
    if ((i + 1) % batch_size == 0) {
      LOG_INST.info(Loc::current(), "Processed ", (i + 1), " nets", stage_monitor.getStatsInfo());
    }
  }

  LOG_INST.info(Loc::current(), "Processed ", egr_net_list.size(), " nets", monitor.getStatsInfo());
}

void EarlyGlobalRouter::routeEGRNet(EGRNet& egr_net)
{
  if (skipRouting(egr_net)) {
    return;
  }
  EGRRoutingPackage egr_routing_package = initEGRRoutingPackage(egr_net);
  routeEGRRoutingPackage(egr_routing_package);
  updateRoutingSegmentList(egr_net, egr_routing_package);
  updateLayerResourceMap(egr_net);
}

bool EarlyGlobalRouter::skipRouting(EGRNet& egr_net)
{
  return RTUtil::exist(_egr_data_manager.getConfig().skip_net_name_set, egr_net.get_net_name());
}

EGRRoutingPackage EarlyGlobalRouter::initEGRRoutingPackage(EGRNet& egr_net)
{
  EGRRoutingPackage egr_routing_package;

  std::vector<LayerCoord>& pin_coord_list = egr_routing_package.get_pin_coord_list();
  for (EGRPin& egr_pin : egr_net.get_pin_list()) {
    pin_coord_list.push_back(egr_pin.getGridCoordList().front());
  }
  std::sort(pin_coord_list.begin(), pin_coord_list.end(), CmpLayerCoordByXASC());
  pin_coord_list.erase(std::unique(pin_coord_list.begin(), pin_coord_list.end()), pin_coord_list.end());

  std::vector<Segment<LayerCoord>>& routing_segment_list = egr_routing_package.get_routing_segment_list();
  LayerCoord driving_pin_grid_coord = egr_net.get_driving_pin().getGridCoordList().front();
  routing_segment_list.emplace_back(driving_pin_grid_coord, driving_pin_grid_coord);

  std::map<LayerCoord, std::pair<irt_int, LayerCoord>, CmpLayerCoordByXASC>& min_distance_map = egr_routing_package.get_min_distance_map();
  for (LayerCoord& pin_coord : pin_coord_list) {
    min_distance_map[pin_coord] = make_pair(INT_MAX, LayerCoord());
  }
  egr_routing_package.set_number_already_counted(0);
  return egr_routing_package;
}

void EarlyGlobalRouter::routeEGRRoutingPackage(EGRRoutingPackage& egr_routing_package)
{
  EGRStrategy strategy = _egr_data_manager.getConfig().egr_strategy;
  if (strategy == EGRStrategy::kTopo) {
    routeByTopo(egr_routing_package);
  } else if (strategy == EGRStrategy::kGradul) {
    routeByGradual(egr_routing_package);
  }
}

void EarlyGlobalRouter::routeByTopo(EGRRoutingPackage& egr_routing_package)
{
  generateTopoSegmentPairList(egr_routing_package);
  generateTopoCoordPairList(egr_routing_package);
  routeAllCoordPairs(egr_routing_package);
}

void EarlyGlobalRouter::generateTopoSegmentPairList(EGRRoutingPackage& egr_routing_package)
{
  std::vector<std::pair<Segment<LayerCoord>, Segment<LayerCoord>>>& topo_segment_pair_list
      = egr_routing_package.get_topo_segment_pair_list();
  irt_int bottom_routing_layer_idx = _egr_data_manager.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = _egr_data_manager.getConfig().top_routing_layer_idx;
  std::vector<LayerCoord>& pin_coord_list = egr_routing_package.get_pin_coord_list();

  static std::once_flag init_flag;
  std::call_once(init_flag, Flute::readLUT);

  size_t pin_size = pin_coord_list.size();
  Flute::DTYPE x[pin_size];
  Flute::DTYPE y[pin_size];
  irt_int coord_num = 0;
  Segment<LayerCoord> segment_first;
  Segment<LayerCoord> segment_second;
  std::map<PlanarCoord, LayerCoord, CmpPlanarCoordByXASC> planar_layer_coord_map;
  for (LayerCoord& pin_coord : pin_coord_list) {
    if (RTUtil::exist(planar_layer_coord_map, PlanarCoord(pin_coord))) {
      segment_first = Segment<LayerCoord>(planar_layer_coord_map[pin_coord], planar_layer_coord_map[pin_coord]);
      segment_second = Segment<LayerCoord>(pin_coord, pin_coord);
      topo_segment_pair_list.push_back(make_pair(segment_first, segment_second));
      LOG_INST.info(Loc::current(), "Same planar coord exist");
      continue;
    }
    planar_layer_coord_map[pin_coord] = pin_coord;
    x[coord_num] = pin_coord.get_x();
    y[coord_num] = pin_coord.get_y();
    ++coord_num;
  }

  if (coord_num == 1) {
    return;
  }
  Flute::Tree flute_tree = Flute::flute(coord_num, x, y, FLUTE_ACCURACY);
  irt_int coord_pair_size = 2 * flute_tree.deg - 2;
  topo_segment_pair_list.reserve(topo_segment_pair_list.size() + coord_pair_size);
  for (irt_int i = 0; i < coord_pair_size; i++) {
    irt_int n_id = flute_tree.branch[i].n;
    PlanarCoord first_coord(flute_tree.branch[i].x, flute_tree.branch[i].y);
    PlanarCoord second_coord(flute_tree.branch[n_id].x, flute_tree.branch[n_id].y);

    if (first_coord == second_coord) {  // check
      continue;
    }

    Segment<LayerCoord> seg_first;
    Segment<LayerCoord> seg_second;
    if (RTUtil::exist(planar_layer_coord_map, PlanarCoord(first_coord))) {
      seg_first = Segment<LayerCoord>(planar_layer_coord_map[first_coord], planar_layer_coord_map[first_coord]);
    } else {
      seg_first = Segment<LayerCoord>(LayerCoord(first_coord, bottom_routing_layer_idx), LayerCoord(first_coord, top_routing_layer_idx));
    }
    if (RTUtil::exist(planar_layer_coord_map, PlanarCoord(second_coord))) {
      seg_second = Segment<LayerCoord>(planar_layer_coord_map[second_coord], planar_layer_coord_map[second_coord]);
    } else {
      seg_second = Segment<LayerCoord>(LayerCoord(second_coord, bottom_routing_layer_idx), LayerCoord(second_coord, top_routing_layer_idx));
    }

    topo_segment_pair_list.push_back(make_pair(seg_first, seg_second));
  }
}

void EarlyGlobalRouter::generateTopoCoordPairList(EGRRoutingPackage& egr_routing_package)
{
  std::vector<std::pair<Segment<LayerCoord>, Segment<LayerCoord>>>& topo_segment_pair_list
      = egr_routing_package.get_topo_segment_pair_list();
  std::vector<std::pair<irt::LayerCoord, irt::LayerCoord>>& topo_coord_pair_list = egr_routing_package.get_topo_coord_pair_list();

  std::vector<std::pair<Segment<LayerCoord>, Segment<LayerCoord>>> steiner_segment_pair_list
      = egr_routing_package.get_topo_segment_pair_list();

  topo_coord_pair_list.reserve(topo_segment_pair_list.size());
  for (std::pair<Segment<LayerCoord>, Segment<LayerCoord>>& segment_pair : topo_segment_pair_list) {
    Segment<LayerCoord> first_segment = segment_pair.first;
    Segment<LayerCoord> second_segment = segment_pair.second;
    bool first_is_point = (first_segment.get_first() == first_segment.get_second());
    bool second_is_point = (second_segment.get_first() == second_segment.get_second());
    if (first_is_point && second_is_point) {
      topo_coord_pair_list.push_back(make_pair(first_segment.get_first(), second_segment.get_first()));
    } else {
    }
  }
}

void EarlyGlobalRouter::routeAllCoordPairs(EGRRoutingPackage& egr_routing_package)
{
  std::vector<std::pair<LayerCoord, LayerCoord>>& topo_coord_pair_list = egr_routing_package.get_topo_coord_pair_list();
  for (std::pair<LayerCoord, LayerCoord>& coord_pair : topo_coord_pair_list) {
    routeInPattern(egr_routing_package, coord_pair);
  }
}

void EarlyGlobalRouter::routeByGradual(EGRRoutingPackage& egr_routing_package)
{
  while (egr_routing_package.continueRouting()) {
    updateNearestCoordPair(egr_routing_package);
    routeNearestCoordPair(egr_routing_package);
  }
}

void EarlyGlobalRouter::updateNearestCoordPair(EGRRoutingPackage& egr_routing_package)
{
  std::vector<LayerCoord>& pin_coord_list = egr_routing_package.get_pin_coord_list();
  std::vector<Segment<LayerCoord>>& routing_segment_list = egr_routing_package.get_routing_segment_list();
  irt_int number_already_counted = egr_routing_package.get_number_already_counted();
  std::map<LayerCoord, std::pair<irt_int, LayerCoord>, CmpLayerCoordByXASC>& min_distance_map = egr_routing_package.get_min_distance_map();

  for (size_t i = number_already_counted; i < routing_segment_list.size(); i++) {
    for (LayerCoord& pin_coord : pin_coord_list) {
      Segment<LayerCoord>& routing_segment = routing_segment_list[i];
      LayerCoord& first_coord = routing_segment.get_first();
      LayerCoord& second_coord = routing_segment.get_second();
      LayerCoord seg_coord = first_coord;

      if (RTUtil::isHorizontal(first_coord, second_coord)) {
        irt_int first_x = first_coord.get_x();
        irt_int second_x = second_coord.get_x();
        RTUtil::sortASC(first_x, second_x);
        if (first_x < pin_coord.get_x() && pin_coord.get_x() < second_x) {
          seg_coord.set_x(pin_coord.get_x());
        } else if (pin_coord.get_x() <= first_x) {
          seg_coord.set_x(first_x);
        } else if (second_x <= pin_coord.get_x()) {
          seg_coord.set_x(second_x);
        }
      } else if (RTUtil::isVertical(first_coord, second_coord)) {
        irt_int first_y = first_coord.get_y();
        irt_int second_y = second_coord.get_y();
        RTUtil::sortASC(first_y, second_y);
        if (first_y < pin_coord.get_y() && pin_coord.get_y() < second_y) {
          seg_coord.set_y(pin_coord.get_y());
        } else if (pin_coord.get_y() <= first_y) {
          seg_coord.set_y(first_y);
        } else if (second_y <= pin_coord.get_y()) {
          seg_coord.set_y(second_y);
        }
      }
      irt_int distance = RTUtil::getManhattanDistance(pin_coord, seg_coord);
      if (min_distance_map[pin_coord].first > distance) {
        min_distance_map[pin_coord] = make_pair(distance, seg_coord);
      }
    }
  }
  egr_routing_package.set_number_already_counted(static_cast<irt_int>(routing_segment_list.size()));

  irt_int min_distance = IRT_INT_MAX;
  LayerCoord best_pin_coord;
  LayerCoord best_seg_coord;
  for (auto [pin_coord, distance_seg_coord] : min_distance_map) {
    if (distance_seg_coord.first < min_distance) {
      min_distance = distance_seg_coord.first;
      best_pin_coord = pin_coord;
      best_seg_coord = distance_seg_coord.second;
    }
  }
  egr_routing_package.set_pin_coord(best_pin_coord);
  egr_routing_package.set_seg_coord(best_seg_coord);
  min_distance_map.erase(best_pin_coord);
  for (LayerCoord& pin_coord : pin_coord_list) {
    if (pin_coord != best_pin_coord) {
      continue;
    }
    std::swap(pin_coord, pin_coord_list.back());
    pin_coord_list.pop_back();
    break;
  }
}

void EarlyGlobalRouter::routeNearestCoordPair(EGRRoutingPackage& egr_routing_package)
{
  std::pair<LayerCoord, LayerCoord> coord_pair = make_pair(egr_routing_package.get_pin_coord(), egr_routing_package.get_seg_coord());
  routeInPattern(egr_routing_package, coord_pair);
}

void EarlyGlobalRouter::routeInPattern(EGRRoutingPackage& egr_routing_package, std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  double best_path_cost = DBL_MAX;
  std::vector<Segment<LayerCoord>> best_routing_segment_list = {};
  std::vector<std::vector<Segment<LayerCoord>>> routing_segment_comb_list;
  routeByStraight(routing_segment_comb_list, coord_pair);
  routeByLPattern(routing_segment_comb_list, coord_pair);
  bool pass = updateBestSegmentList(routing_segment_comb_list, best_routing_segment_list, best_path_cost);

  if (!pass) {
    routing_segment_comb_list.clear();
    routeByZPattern(routing_segment_comb_list, coord_pair);
    routeByInner3BendsPattern(routing_segment_comb_list, coord_pair);
    pass = updateBestSegmentList(routing_segment_comb_list, best_routing_segment_list, best_path_cost);
  }
  if (!pass) {
    routing_segment_comb_list.clear();
    routeByUPattern(routing_segment_comb_list, coord_pair);
    routeByOuter3BendsPattern(routing_segment_comb_list, coord_pair);
    pass = updateBestSegmentList(routing_segment_comb_list, best_routing_segment_list, best_path_cost);
  }

  std::vector<Segment<LayerCoord>>& routing_segment_list = egr_routing_package.get_routing_segment_list();
  routing_segment_list.insert(routing_segment_list.end(), best_routing_segment_list.begin(), best_routing_segment_list.end());
}

bool EarlyGlobalRouter::updateBestSegmentList(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                              std::vector<Segment<LayerCoord>>& best_routing_segment_list, double& best_path_cost)
{
  irt_int bottom_routing_layer_idx = _egr_data_manager.getConfig().bottom_routing_layer_idx;
  irt_int top_routing_layer_idx = _egr_data_manager.getConfig().top_routing_layer_idx;
  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();

  irt_int comb_size = static_cast<irt_int>(routing_segment_comb_list.size());
  std::vector<int> pass_list(comb_size, 1);
  std::vector<double> cost_list(comb_size, 0);
  std::vector<double> avg_cost_list(comb_size, 0);
#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < routing_segment_comb_list.size(); ++i) {
    std::vector<Segment<LayerCoord>>& routing_segment_list = routing_segment_comb_list[i];
    double& path_cost = cost_list[i];
    int& pass = pass_list[i];
    for (size_t j = 0; j < routing_segment_list.size(); ++j) {
      Segment<LayerCoord>& routing_segment = routing_segment_list[j];
      LayerCoord& first_coord = routing_segment.get_first();
      LayerCoord& second_coord = routing_segment.get_second();
      if (first_coord == second_coord) {
        continue;
      }
      irt_int first_x = first_coord.get_x();
      irt_int first_y = first_coord.get_y();
      irt_int first_layer_idx = first_coord.get_layer_idx();
      irt_int second_x = second_coord.get_x();
      irt_int second_y = second_coord.get_y();
      irt_int second_layer_idx = second_coord.get_layer_idx();
      if (RTUtil::isProximal(first_coord, second_coord)) {
        RTUtil::sortASC(first_layer_idx, second_layer_idx);
        for (irt_int layer_idx = first_layer_idx; layer_idx <= second_layer_idx; ++layer_idx) {
          double node_cost = layer_resource_map[layer_idx][first_x][first_y].getCost(EGRResourceType::kTrack);
          if (layer_idx <= bottom_routing_layer_idx && layer_idx >= top_routing_layer_idx && node_cost >= 1) {
            pass = false;
          }
          path_cost += node_cost;
        }
      } else if (RTUtil::isVertical(first_coord, second_coord)) {
        RTUtil::sortASC(first_y, second_y);
        for (irt_int y = first_y; y <= second_y; ++y) {
          for (EGRResourceType resource_type : {EGRResourceType::kNorth, EGRResourceType::kSouth, EGRResourceType::kTrack}) {
            double node_cost = layer_resource_map[first_layer_idx][first_x][y].getCost(resource_type);
            if (node_cost >= 1) {
              pass = false;
            }
            path_cost += node_cost;
          }
        }
        path_cost -= layer_resource_map[first_layer_idx][first_x][first_y].getCost(EGRResourceType::kSouth);
        path_cost -= layer_resource_map[first_layer_idx][first_x][second_y].getCost(EGRResourceType::kNorth);
      } else if (RTUtil::isHorizontal(first_coord, second_coord)) {
        RTUtil::sortASC(first_x, second_x);
        for (irt_int x = first_x; x <= second_x; ++x) {
          for (EGRResourceType resource_type : {EGRResourceType::kWest, EGRResourceType::kEast, EGRResourceType::kTrack}) {
            double node_cost = layer_resource_map[first_layer_idx][x][first_y].getCost(resource_type);
            if (node_cost >= 1) {
              pass = false;
            }
            path_cost += node_cost;
          }
        }
        path_cost -= layer_resource_map[first_layer_idx][first_x][first_y].getCost(EGRResourceType::kWest);
        path_cost -= layer_resource_map[first_layer_idx][second_x][first_y].getCost(EGRResourceType::kEast);
      } else {
        LOG_INST.error(Loc::current(), "The segment is oblique!");
      }
      path_cost += RTUtil::getManhattanDistance(first_coord, second_coord);
    }
  }

  double min_path_cost = DBL_MAX;
  irt_int best_path_idx = -1;
  for (int i = 0; i < comb_size; i++) {
    if (pass_list[i] == 1) {
      best_routing_segment_list = routing_segment_comb_list[i];
      return true;
    }
    if (cost_list[i] < min_path_cost) {
      best_path_idx = i;
      min_path_cost = cost_list[i];
    }
  }
  if (min_path_cost < best_path_cost) {
    best_routing_segment_list = routing_segment_comb_list[best_path_idx];
    best_path_cost = min_path_cost;
  }
  return false;
}

void EarlyGlobalRouter::routeByStraight(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                        std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;
  if (RTUtil::isOblique(start_coord, end_coord)) {
    return;
  }
  if (RTUtil::isProximal(start_coord, end_coord)) {
    routing_segment_comb_list.push_back({Segment<LayerCoord>(start_coord, end_coord)});
    return;
  }
  EGRDatabase& database = _egr_data_manager.getDatabase();
  std::vector<irt_int> candidate_layer_idx_list;
  if (RTUtil::isHorizontal(start_coord, end_coord)) {
    candidate_layer_idx_list = database.get_h_layer_idx_list();
  } else {
    candidate_layer_idx_list = database.get_v_layer_idx_list();
  }
  for (irt_int candidate_layer_idx : candidate_layer_idx_list) {
    LayerCoord inflection_coord1(start_coord.get_planar_coord(), candidate_layer_idx);
    LayerCoord inflection_coord2(end_coord.get_planar_coord(), candidate_layer_idx);
    routing_segment_comb_list.push_back({Segment<LayerCoord>(start_coord, inflection_coord1),
                                         Segment<LayerCoord>(inflection_coord1, inflection_coord2),
                                         Segment<LayerCoord>(inflection_coord2, end_coord)});
  }
}

void EarlyGlobalRouter::routeByLPattern(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                        std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;

  if (RTUtil::isRightAngled(start_coord, end_coord)) {
    return;
  }
  EGRDatabase& database = _egr_data_manager.getDatabase();
  for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
    for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
      LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
      LayerCoord inflection_coord2(start_coord.get_x(), end_coord.get_y(), v_layer_idx);
      LayerCoord inflection_coord3(start_coord.get_x(), end_coord.get_y(), h_layer_idx);
      LayerCoord inflection_coord4(end_coord.get_planar_coord(), h_layer_idx);
      routing_segment_comb_list.push_back(
          {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
           Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
           Segment<LayerCoord>(inflection_coord4, end_coord)});
    }
  }

  for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
    for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
      LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
      LayerCoord inflection_coord2(end_coord.get_x(), start_coord.get_y(), h_layer_idx);
      LayerCoord inflection_coord3(end_coord.get_x(), start_coord.get_y(), v_layer_idx);
      LayerCoord inflection_coord4(end_coord.get_planar_coord(), v_layer_idx);
      routing_segment_comb_list.push_back(
          {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
           Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
           Segment<LayerCoord>(inflection_coord4, end_coord)});
    }
  }
}

void EarlyGlobalRouter::routeByUPattern(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                        std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  EGRDatabase& egr_database = _egr_data_manager.getDatabase();
  PlanarRect die = egr_database.get_die().get_grid_rect();
  irt_int die_lb_x = egr_database.get_die().get_grid_lb_x();
  irt_int die_lb_y = egr_database.get_die().get_grid_lb_y();
  irt_int die_rt_x = egr_database.get_die().get_grid_rt_x();
  irt_int die_rt_y = egr_database.get_die().get_grid_rt_y();

  irt_int scope = 2 * _egr_data_manager.getConfig().accuracy;
  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;
  irt_int start_x = start_coord.get_x();
  irt_int end_x = end_coord.get_x();
  irt_int start_y = start_coord.get_y();
  irt_int end_y = end_coord.get_y();
  RTUtil::sortASC(start_x, end_x);
  RTUtil::sortASC(start_y, end_y);

  if (RTUtil::isProximal(start_coord, end_coord)) {
    return;
  }

  std::vector<irt_int> inflection_x_list;
  std::vector<irt_int> inflection_y_list;
  for (int i = 1; i <= scope; ++i) {
    if (!RTUtil::isHorizontal(start_coord, end_coord)) {
      if (start_x - i > die_lb_x) {
        inflection_x_list.push_back(start_x - i);
      }
      if (end_x + i < die_rt_x) {
        inflection_x_list.push_back(end_x + i);
      }
    }
    if (!RTUtil::isVertical(start_coord, end_coord)) {
      if (start_y - i > die_lb_y) {
        inflection_y_list.push_back(start_y - i);
      }
      if (end_y + i < die_rt_y) {
        inflection_y_list.push_back(end_y + i);
      }
    }
  }
  for (irt_int inflection_x : inflection_x_list) {
    for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
      for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
        LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
        LayerCoord inflection_coord2(inflection_x, start_coord.get_y(), h_layer_idx);
        LayerCoord inflection_coord3(inflection_x, start_coord.get_y(), v_layer_idx);
        LayerCoord inflection_coord4(inflection_x, end_coord.get_y(), v_layer_idx);
        LayerCoord inflection_coord5(inflection_x, end_coord.get_y(), h_layer_idx);
        LayerCoord inflection_coord6(end_coord.get_planar_coord(), h_layer_idx);
        routing_segment_comb_list.push_back(
            {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
             Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
             Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
             Segment<LayerCoord>(inflection_coord6, end_coord)});
      }
    }
  }

  for (irt_int inflection_y : inflection_y_list) {
    for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
      for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
        LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
        LayerCoord inflection_coord2(start_coord.get_x(), inflection_y, v_layer_idx);
        LayerCoord inflection_coord3(start_coord.get_x(), inflection_y, h_layer_idx);
        LayerCoord inflection_coord4(end_coord.get_x(), inflection_y, h_layer_idx);
        LayerCoord inflection_coord5(end_coord.get_x(), inflection_y, v_layer_idx);
        LayerCoord inflection_coord6(end_coord.get_planar_coord(), v_layer_idx);
        routing_segment_comb_list.push_back(
            {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
             Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
             Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
             Segment<LayerCoord>(inflection_coord6, end_coord)});
      }
    }
  }
}

void EarlyGlobalRouter::routeByZPattern(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                        std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  EGRDatabase& database = _egr_data_manager.getDatabase();
  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;

  if (RTUtil::isRightAngled(start_coord, end_coord)) {
    return;
  }
  std::vector<irt_int> x_mid_index_list = getMidIndexList(start_coord.get_x(), end_coord.get_x());
  std::vector<irt_int> y_mid_index_list = getMidIndexList(start_coord.get_y(), end_coord.get_y());
  if (x_mid_index_list.empty() && y_mid_index_list.empty()) {
    return;
  }
  for (size_t i = 0; i < x_mid_index_list.size(); i++) {
    for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
      for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
        LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
        LayerCoord inflection_coord2(x_mid_index_list[i], start_coord.get_y(), h_layer_idx);
        LayerCoord inflection_coord3(x_mid_index_list[i], start_coord.get_y(), v_layer_idx);
        LayerCoord inflection_coord4(x_mid_index_list[i], end_coord.get_y(), v_layer_idx);
        LayerCoord inflection_coord5(x_mid_index_list[i], end_coord.get_y(), h_layer_idx);
        LayerCoord inflection_coord6(end_coord.get_planar_coord(), h_layer_idx);
        routing_segment_comb_list.push_back(
            {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
             Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
             Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
             Segment<LayerCoord>(inflection_coord6, end_coord)});
      }
    }
  }
  for (size_t i = 0; i < y_mid_index_list.size(); i++) {
    for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
      for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
        LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
        LayerCoord inflection_coord2(start_coord.get_x(), y_mid_index_list[i], v_layer_idx);
        LayerCoord inflection_coord3(start_coord.get_x(), y_mid_index_list[i], h_layer_idx);
        LayerCoord inflection_coord4(end_coord.get_x(), y_mid_index_list[i], h_layer_idx);
        LayerCoord inflection_coord5(end_coord.get_x(), y_mid_index_list[i], v_layer_idx);
        LayerCoord inflection_coord6(end_coord.get_planar_coord(), v_layer_idx);
        routing_segment_comb_list.push_back(
            {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
             Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
             Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
             Segment<LayerCoord>(inflection_coord6, end_coord)});
      }
    }
  }
}

std::vector<irt_int> EarlyGlobalRouter::getMidIndexList(irt_int start_idx, irt_int end_idx)
{
  std::vector<irt_int> index_list;
  RTUtil::sortASC(start_idx, end_idx);
  irt_int interval = (end_idx - start_idx - 1) / (_egr_data_manager.getConfig().accuracy + 1) + 1;
  for (irt_int i = (start_idx + interval); i <= (end_idx - 1); i += interval) {
    index_list.push_back(i);
  }
  return index_list;
}

void EarlyGlobalRouter::routeByInner3BendsPattern(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                                  std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;

  if (RTUtil::isRightAngled(start_coord, end_coord)) {
    return;
  }
  EGRDatabase& database = _egr_data_manager.getDatabase();
  std::vector<irt_int> x_mid_index_list = getMidIndexList(start_coord.get_x(), end_coord.get_x());
  std::vector<irt_int> y_mid_index_list = getMidIndexList(start_coord.get_y(), end_coord.get_y());
  if (x_mid_index_list.empty() || y_mid_index_list.empty()) {
    return;
  }

  for (size_t i = 0; i < x_mid_index_list.size(); i++) {
    for (size_t j = 0; j < y_mid_index_list.size(); j++) {
      for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
          LayerCoord inflection_coord2(x_mid_index_list[i], start_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord3(x_mid_index_list[i], start_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord4(x_mid_index_list[i], y_mid_index_list[j], v_layer_idx);
          LayerCoord inflection_coord5(x_mid_index_list[i], y_mid_index_list[j], h_layer_idx);
          LayerCoord inflection_coord6(end_coord.get_x(), y_mid_index_list[j], h_layer_idx);
          LayerCoord inflection_coord7(end_coord.get_x(), y_mid_index_list[j], v_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), v_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
  }

  for (size_t i = 0; i < x_mid_index_list.size(); i++) {
    for (size_t j = 0; j < y_mid_index_list.size(); j++) {
      for (irt_int h_layer_idx : database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
          LayerCoord inflection_coord2(start_coord.get_x(), y_mid_index_list[j], v_layer_idx);
          LayerCoord inflection_coord3(start_coord.get_x(), y_mid_index_list[j], h_layer_idx);
          LayerCoord inflection_coord4(x_mid_index_list[i], y_mid_index_list[j], h_layer_idx);
          LayerCoord inflection_coord5(x_mid_index_list[i], y_mid_index_list[j], v_layer_idx);
          LayerCoord inflection_coord6(x_mid_index_list[i], end_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord7(x_mid_index_list[i], end_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), h_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
  }
}

void EarlyGlobalRouter::routeByOuter3BendsPattern(std::vector<std::vector<Segment<LayerCoord>>>& routing_segment_comb_list,
                                                  std::pair<LayerCoord, LayerCoord>& coord_pair)
{
  EGRDatabase& egr_database = _egr_data_manager.getDatabase();
  PlanarRect die = egr_database.get_die().get_grid_rect();
  irt_int die_lb_x = egr_database.get_die().get_grid_lb_x();
  irt_int die_lb_y = egr_database.get_die().get_grid_lb_y();
  irt_int die_rt_x = egr_database.get_die().get_grid_rt_x();
  irt_int die_rt_y = egr_database.get_die().get_grid_rt_y();

  LayerCoord start_coord = coord_pair.first;
  LayerCoord end_coord = coord_pair.second;

  if (RTUtil::isProximal(start_coord, end_coord)) {
    routing_segment_comb_list.push_back({Segment<LayerCoord>(start_coord, end_coord)});
    return;
  }
  if (start_coord.get_x() > end_coord.get_x()) {
    std::swap(start_coord, end_coord);
  }

  irt_int scope = 2 * _egr_data_manager.getConfig().accuracy;
  if (start_coord.get_y() <= end_coord.get_y()) {
    for (irt_int i = 1; i <= scope; ++i) {
      irt_int inflection_x = start_coord.get_x() - i;
      irt_int inflection_y = end_coord.get_y() + i;
      if ((inflection_x < die_lb_x) || (inflection_y > die_rt_y)) {
        break;
      }
      for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
          LayerCoord inflection_coord2(inflection_x, start_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord3(inflection_x, start_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord4(inflection_x, inflection_y, v_layer_idx);
          LayerCoord inflection_coord5(inflection_x, inflection_y, h_layer_idx);
          LayerCoord inflection_coord6(end_coord.get_x(), inflection_y, h_layer_idx);
          LayerCoord inflection_coord7(end_coord.get_x(), inflection_y, v_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), v_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
    for (irt_int i = 1; i <= scope; ++i) {
      irt_int inflection_x = end_coord.get_x() + i;
      irt_int inflection_y = start_coord.get_y() - i;
      if ((inflection_x > die_rt_x) || (inflection_y < die_lb_y)) {
        break;
      }
      for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
          LayerCoord inflection_coord2(start_coord.get_x(), inflection_y, v_layer_idx);
          LayerCoord inflection_coord3(start_coord.get_x(), inflection_y, h_layer_idx);
          LayerCoord inflection_coord4(inflection_x, inflection_y, h_layer_idx);
          LayerCoord inflection_coord5(inflection_x, inflection_y, v_layer_idx);
          LayerCoord inflection_coord6(inflection_x, end_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord7(inflection_x, end_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), h_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
  }

  if (start_coord.get_y() >= end_coord.get_y()) {
    for (irt_int i = 1; i <= scope; ++i) {
      irt_int inflection_x = start_coord.get_x() - i;
      irt_int inflection_y = end_coord.get_y() - i;
      if ((inflection_x < die_lb_x) || (inflection_y < die_lb_y)) {
        break;
      }
      for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), h_layer_idx);
          LayerCoord inflection_coord2(inflection_x, start_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord3(inflection_x, start_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord4(inflection_x, inflection_y, v_layer_idx);
          LayerCoord inflection_coord5(inflection_x, inflection_y, h_layer_idx);
          LayerCoord inflection_coord6(end_coord.get_x(), inflection_y, h_layer_idx);
          LayerCoord inflection_coord7(end_coord.get_x(), inflection_y, v_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), v_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
    for (irt_int i = 1; i <= scope; ++i) {
      irt_int inflection_x = end_coord.get_x() + i;
      irt_int inflection_y = start_coord.get_y() + i;
      if ((inflection_x > die_rt_x) || (inflection_y > die_rt_y)) {
        break;
      }
      for (irt_int h_layer_idx : egr_database.get_h_layer_idx_list()) {
        for (irt_int v_layer_idx : egr_database.get_v_layer_idx_list()) {
          LayerCoord inflection_coord1(start_coord.get_planar_coord(), v_layer_idx);
          LayerCoord inflection_coord2(start_coord.get_x(), inflection_y, v_layer_idx);
          LayerCoord inflection_coord3(start_coord.get_x(), inflection_y, h_layer_idx);
          LayerCoord inflection_coord4(inflection_x, inflection_y, h_layer_idx);
          LayerCoord inflection_coord5(inflection_x, inflection_y, v_layer_idx);
          LayerCoord inflection_coord6(inflection_x, end_coord.get_y(), v_layer_idx);
          LayerCoord inflection_coord7(inflection_x, end_coord.get_y(), h_layer_idx);
          LayerCoord inflection_coord8(end_coord.get_planar_coord(), h_layer_idx);
          routing_segment_comb_list.push_back(
              {Segment<LayerCoord>(start_coord, inflection_coord1), Segment<LayerCoord>(inflection_coord1, inflection_coord2),
               Segment<LayerCoord>(inflection_coord2, inflection_coord3), Segment<LayerCoord>(inflection_coord3, inflection_coord4),
               Segment<LayerCoord>(inflection_coord4, inflection_coord5), Segment<LayerCoord>(inflection_coord5, inflection_coord6),
               Segment<LayerCoord>(inflection_coord6, inflection_coord7), Segment<LayerCoord>(inflection_coord7, inflection_coord8),
               Segment<LayerCoord>(inflection_coord8, end_coord)});
        }
      }
    }
  }
}

void EarlyGlobalRouter::updateRoutingSegmentList(EGRNet& egr_net, EGRRoutingPackage& egr_routing_package)
{
  std::vector<LayerCoord> candidate_root_coord_list = egr_net.get_driving_pin().getGridCoordList();
  std::vector<Segment<LayerCoord>>& routing_segment_list = egr_routing_package.get_routing_segment_list();

  std::map<LayerCoord, std::set<irt_int>, CmpLayerCoordByXASC> key_coord_pin_map;
  for (EGRPin& egr_pin : egr_net.get_pin_list()) {
    for (LayerCoord& grid_coord : egr_pin.getGridCoordList()) {
      key_coord_pin_map[grid_coord].insert(egr_pin.get_pin_idx());
    }
  }
  egr_net.set_coord_tree(RTUtil::getTreeByFullFlow(candidate_root_coord_list, routing_segment_list, key_coord_pin_map));
}

void EarlyGlobalRouter::updateLayerResourceMap(EGRNet& egr_net)
{
  MTree<LayerCoord>& coord_tree = egr_net.get_coord_tree();
  std::vector<Segment<TNode<LayerCoord>*>> routing_segment_list = RTUtil::getSegListByTree(coord_tree);

  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();
  if (routing_segment_list.empty()) {
    LayerCoord driving_pin_grid_coord = egr_net.get_driving_pin().getGridCoordList().front();
    irt_int layer_idx = driving_pin_grid_coord.get_layer_idx();
    irt_int x = driving_pin_grid_coord.get_x();
    irt_int y = driving_pin_grid_coord.get_y();
    layer_resource_map[layer_idx][x][y].addDemand(EGRResourceType::kTrack, 1);
    return;
  }
  addDemandBySegmentList(routing_segment_list);
}

void EarlyGlobalRouter::addDemandBySegmentList(std::vector<Segment<TNode<LayerCoord>*>>& segment_list)
{
  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();

  double wire_demand = 1;
  double half_wire_demand = 0.5;
  double via_demand = 0.2;

  std::set<LayerCoord, CmpLayerCoordByXASC> via_coord_set;
  for (Segment<TNode<LayerCoord>*>& segment : segment_list) {
    LayerCoord& first_coord = segment.get_first()->value();
    LayerCoord& second_coord = segment.get_second()->value();
    irt_int first_x = first_coord.get_x();
    irt_int first_y = first_coord.get_y();
    irt_int first_layer_idx = first_coord.get_layer_idx();
    irt_int second_x = second_coord.get_x();
    irt_int second_y = second_coord.get_y();
    irt_int second_layer_idx = second_coord.get_layer_idx();

    if (RTUtil::isProximal(first_coord, second_coord)) {
      RTUtil::sortASC(first_layer_idx, second_layer_idx);
      for (irt_int layer_idx = first_layer_idx; layer_idx <= second_layer_idx; ++layer_idx) {
        LayerCoord via_coord(first_x, first_y, layer_idx);
        if (RTUtil::exist(via_coord_set, via_coord)) {
          continue;
        }
        layer_resource_map[layer_idx][first_x][first_y].addDemand(EGRResourceType::kTrack, via_demand);
        via_coord_set.insert(via_coord);
      }
    } else if (RTUtil::isVertical(first_coord, second_coord)) {
      GridMap<EGRNode>& resource_map = layer_resource_map[first_layer_idx];
      RTUtil::sortASC(first_y, second_y);
      for (irt_int y = first_y; y <= second_y; ++y) {
        for (EGRResourceType resource_type : {EGRResourceType::kTrack, EGRResourceType::kNorth, EGRResourceType::kSouth}) {
          resource_map[first_x][y].addDemand(resource_type, wire_demand);
        }
      }
      resource_map[first_x][first_y].addDemand(EGRResourceType::kSouth, -1 * wire_demand);
      resource_map[first_x][first_y].addDemand(EGRResourceType::kTrack, -1 * half_wire_demand);
      resource_map[first_x][second_y].addDemand(EGRResourceType::kNorth, -1 * wire_demand);
      resource_map[first_x][second_y].addDemand(EGRResourceType::kTrack, -1 * half_wire_demand);
    } else if (RTUtil::isHorizontal(first_coord, second_coord)) {
      GridMap<EGRNode>& resource_map = layer_resource_map[first_layer_idx];
      RTUtil::sortASC(first_x, second_x);
      for (irt_int x = first_x; x <= second_x; ++x) {
        for (EGRResourceType resource_type : {EGRResourceType::kTrack, EGRResourceType::kWest, EGRResourceType::kEast}) {
          resource_map[x][first_y].addDemand(resource_type, wire_demand);
        }
      }
      resource_map[first_x][first_y].addDemand(EGRResourceType::kWest, -1 * wire_demand);
      resource_map[first_x][first_y].addDemand(EGRResourceType::kTrack, -1 * half_wire_demand);
      resource_map[second_x][first_y].addDemand(EGRResourceType::kEast, -1 * wire_demand);
      resource_map[second_x][first_y].addDemand(EGRResourceType::kTrack, -1 * half_wire_demand);
    } else {
      LOG_INST.error(Loc::current(), "The segment is oblique!");
    }
  }
}

void EarlyGlobalRouter::reportEGRNetList()
{
  reportCongestion();
  reportWireViaStatistics();
}

void EarlyGlobalRouter::reportCongestion()
{
  std::vector<GridMap<EGRNode>>& layer_resource_map = _egr_data_manager.getDatabase().get_layer_resource_map();
  std::vector<RoutingLayer>& routing_layer_list = _egr_data_manager.getDatabase().get_routing_layer_list();

  std::vector<std::map<irt_int, irt_int, std::greater<int>>> layer_overflow_map;
  std::map<irt_int, irt_int, std::greater<int>> total_overflow_map;
  layer_overflow_map.resize(routing_layer_list.size());
  irt_int total_overflow = 0;

  std::vector<EGRResourceType> resource_types(
      {EGRResourceType::kNorth, EGRResourceType::kSouth, EGRResourceType::kWest, EGRResourceType::kEast});
  irt_int cell_num
      = layer_resource_map.front().get_x_size() * layer_resource_map.front().get_y_size() * static_cast<irt_int>(resource_types.size());

  // statistics
  for (size_t layer_idx = 0; layer_idx < layer_resource_map.size(); ++layer_idx) {
    GridMap<EGRNode>& resource_map = layer_resource_map[layer_idx];
    std::map<irt_int, irt_int, std::greater<int>>& overflow_map = layer_overflow_map[layer_idx];
    for (irt_int x = 0; x < resource_map.get_x_size(); ++x) {
      for (irt_int y = 0; y < resource_map.get_y_size(); ++y) {
        EGRNode& resource_node = resource_map[x][y];
        irt_int grid_overflow = 0;
        for (EGRResourceType resource_type : resource_types) {
          irt_int overflow = static_cast<irt_int>(std::ceil(resource_node.getOverflow(resource_type)));
          overflow_map[overflow]++;
          total_overflow_map[overflow]++;
          if (overflow > 0) {
            grid_overflow = max(grid_overflow, overflow);
          }
        }
        total_overflow += grid_overflow;
      }
    }
  }

  std::vector<irt_int> overflow_num_list;
  for (auto [overflow_num, quantity] : total_overflow_map) {
    overflow_num_list.push_back(overflow_num);
  }

  fort::char_table table;
  table.set_border_style(FT_SOLID_STYLE);
  // report header
  char c_buffer[1024] = {0};
  table << fort::header << "layer\\overflow";
  for (irt_int i : overflow_num_list) {
    table << i;
  }
  table << "total congestion" << fort::endr;

  irt_int h_layer_num = 0;
  irt_int v_layer_num = 0;
  irt_int h_overflow = 0;
  irt_int v_overflow = 0;
  // report every layer
  for (size_t layer_idx = 0; layer_idx < layer_resource_map.size(); ++layer_idx) {
    std::map<irt_int, irt_int, std::greater<int>>& overflow_map = layer_overflow_map[layer_idx];
    table << routing_layer_list[layer_idx].get_layer_name();
    for (irt_int i : overflow_num_list) {
      sprintf(c_buffer, "%d(%.2f%%)", overflow_map[i], overflow_map[i] * 100.0 / cell_num);
      table << c_buffer;
    }
    // total congestion in one layer
    irt_int layer_total_congestion = 0;
    for (irt_int i = 1; i <= total_overflow_map.begin()->first; ++i) {
      layer_total_congestion += overflow_map[i];
      if (routing_layer_list[layer_idx].isPreferH()) {
        h_overflow += overflow_map[i];
      } else {
        v_overflow += overflow_map[i];
      }
    }
    sprintf(c_buffer, "%d(%.2f%%)", layer_total_congestion, layer_total_congestion * 100.0 / cell_num);
    table << c_buffer << fort::endr;

    if (routing_layer_list[layer_idx].isPreferH()) {
      ++h_layer_num;
    } else {
      ++v_layer_num;
    }
  }
  table << "Total";
  for (irt_int i : overflow_num_list) {
    sprintf(c_buffer, "%d(%.2f%%)", total_overflow_map[i],
            total_overflow_map[i] * 100.0 / cell_num / static_cast<irt_int>(routing_layer_list.size()));
    table << c_buffer;
  }
  irt_int total_congestion = 0;
  for (irt_int i = 1; i <= total_overflow_map.begin()->first; ++i) {
    total_congestion += total_overflow_map[i];
  }
  sprintf(c_buffer, "%d(%.2f%%)", total_congestion, total_congestion * 100.0 / cell_num / static_cast<irt_int>(routing_layer_list.size()));
  table << c_buffer << fort::endr;
  for (std::string table_str : RTUtil::splitString(table.to_string(), '\n')) {
    LOG_INST.info(Loc::current(), table_str);
  }

  // report HV and total overflow
  sprintf(c_buffer, "Overflow Edge num after earlyGlobalRoute is :%d(%5.2f%%)H + %d(%5.2f%%)V", h_overflow,
          h_overflow * 100.0 / h_layer_num / cell_num, v_overflow, v_overflow * 100.0 / v_layer_num / cell_num);
  LOG_INST.info(Loc::current(), c_buffer);
  LOG_INST.info(Loc::current(), "Total overflow is : ", total_overflow);
}

void EarlyGlobalRouter::reportWireViaStatistics()
{
  EGRDatabase& egr_database = _egr_data_manager.getDatabase();
  irt_int cell_width = _egr_data_manager.getConfig().cell_width;
  irt_int cell_height = _egr_data_manager.getConfig().cell_height;

  for (EGRNet& egr_net : egr_database.get_egr_net_list()) {
    MTree<LayerCoord>& coord_tree = egr_net.get_coord_tree();
    std::vector<Segment<TNode<LayerCoord>*>> routing_segment_list = RTUtil::getSegListByTree(coord_tree);

    if (routing_segment_list.empty()) {
      egr_database.addWireLength((cell_width + cell_height) / 2.0);
    }
    for (Segment<TNode<LayerCoord>*>& segment : routing_segment_list) {
      LayerCoord& first_coord = segment.get_first()->value();
      LayerCoord& second_coord = segment.get_second()->value();
      if (RTUtil::isProximal(first_coord, second_coord)) {
        egr_database.addViaNum(std::abs(first_coord.get_layer_idx() - second_coord.get_layer_idx()));
      } else if (RTUtil::isVertical(first_coord, second_coord) || RTUtil::isHorizontal(first_coord, second_coord)) {
        egr_database.addWireLength(std::abs(first_coord.get_x() - second_coord.get_x()) * cell_width
                                   + std::abs(first_coord.get_y() - second_coord.get_y()) * cell_height);
      } else {
        LOG_INST.error(Loc::current(), "The segment is oblique!");
      }
    }
  }
  LOG_INST.info(Loc::current(), "Wire length: ", static_cast<long long int>(egr_database.get_total_wire_length()));
  LOG_INST.info(Loc::current(), "Via num: ", static_cast<long long int>(egr_database.get_total_via_num()));
}

}  // namespace irt
