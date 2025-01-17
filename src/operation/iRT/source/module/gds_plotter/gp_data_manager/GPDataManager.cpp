#include "GPDataManager.hpp"

#include "GPDataType.hpp"
#include "GPLYPLayer.hpp"
#include "RTUtil.hpp"

namespace irt {

// public

void GPDataManager::input(Config& config, Database& database)
{
  wrapConfig(config);
  wrapDatabase(database);
  buildConfig();
  buildDatabase();
}

// private

void GPDataManager::wrapConfig(Config& config)
{
  _gp_config.temp_directory_path = config.gp_temp_directory_path;
}

void GPDataManager::wrapDatabase(Database& database)
{
  wrapGCellAxis(database);
  wrapDie(database);
  wrapViaLib(database);
  wrapLayerList(database);
  wrapBlockageList(database);
}

void GPDataManager::wrapGCellAxis(Database& database)
{
  GCellAxis& gcell_axis = _gp_database.get_gcell_axis();
  gcell_axis = database.get_gcell_axis();
}

void GPDataManager::wrapDie(Database& database)
{
  EXTPlanarRect& die = _gp_database.get_die();
  die = database.get_die();
}

void GPDataManager::wrapViaLib(Database& database)
{
  std::vector<std::vector<ViaMaster>>& layer_via_master_list = _gp_database.get_layer_via_master_list();
  layer_via_master_list = database.get_layer_via_master_list();
}

void GPDataManager::wrapLayerList(Database& database)
{
  _gp_database.get_routing_layer_list() = database.get_routing_layer_list();
  _gp_database.get_cut_layer_list() = database.get_cut_layer_list();
}

void GPDataManager::wrapBlockageList(Database& database)
{
  _gp_database.get_routing_blockage_list() = database.get_routing_blockage_list();
  _gp_database.get_cut_blockage_list() = database.get_cut_blockage_list();
}

void GPDataManager::buildConfig()
{
}

void GPDataManager::buildDatabase()
{
  buildGDSLayerMap();
  buildLypFile();
}

void GPDataManager::buildGDSLayerMap()
{
  std::map<irt_int, irt_int>& routing_layer_gds_map = _gp_database.get_routing_layer_gds_map();
  std::map<irt_int, irt_int>& gds_routing_layer_map = _gp_database.get_gds_routing_layer_map();
  std::map<irt_int, irt_int>& cut_layer_gds_map = _gp_database.get_cut_layer_gds_map();
  std::map<irt_int, irt_int>& gds_cut_layer_map = _gp_database.get_gds_cut_layer_map();

  std::map<irt_int, irt_int> order_gds_map;
  for (RoutingLayer& routing_layer : _gp_database.get_routing_layer_list()) {
    order_gds_map[routing_layer.get_layer_order()] = -1;
  }
  for (CutLayer& cut_layer : _gp_database.get_cut_layer_list()) {
    order_gds_map[cut_layer.get_layer_order()] = -1;
  }
  // 0为die 最后一个为GCell 中间为cut+routing
  irt_int gds_layer_idx = 1;
  for (auto it = order_gds_map.begin(); it != order_gds_map.end(); it++) {
    it->second = gds_layer_idx++;
  }
  for (RoutingLayer& routing_layer : _gp_database.get_routing_layer_list()) {
    irt_int gds_layer_idx = order_gds_map[routing_layer.get_layer_order()];
    routing_layer_gds_map[routing_layer.get_layer_idx()] = gds_layer_idx;
    gds_routing_layer_map[gds_layer_idx] = routing_layer.get_layer_idx();
  }
  for (CutLayer& cut_layer : _gp_database.get_cut_layer_list()) {
    irt_int gds_layer_idx = order_gds_map[cut_layer.get_layer_order()];
    cut_layer_gds_map[cut_layer.get_layer_idx()] = gds_layer_idx;
    gds_cut_layer_map[gds_layer_idx] = cut_layer.get_layer_idx();
  }
}

void GPDataManager::buildLypFile()
{
  std::vector<RoutingLayer>& routing_layer_list = _gp_database.get_routing_layer_list();
  std::vector<CutLayer>& cut_layer_list = _gp_database.get_cut_layer_list();
  std::map<irt_int, irt_int>& gds_routing_layer_map = _gp_database.get_gds_routing_layer_map();
  std::map<irt_int, irt_int>& gds_cut_layer_map = _gp_database.get_gds_cut_layer_map();

  std::vector<std::string> color_list = {"#808000", "#ff80a8"};
  std::vector<std::string> pattern_list = {"I5", "I9"};

  std::vector<GPDataType> routing_data_type_list
      = {GPDataType::kText,      GPDataType::kBoundingBox, GPDataType::kPort,           GPDataType::kAccessPoint,
         GPDataType::kGuide,     GPDataType::kPreferTrack, GPDataType::kNonpreferTrack, GPDataType::kWire,
         GPDataType::kEnclosure, GPDataType::kBlockage,    GPDataType::kConnection};
  std::vector<GPDataType> cut_data_type_list = {GPDataType::kText, GPDataType::kCut, GPDataType::kBlockage};

  // 0为die 最后一个为GCell 中间为cut+routing
  irt_int gds_layer_size = static_cast<irt_int>(gds_routing_layer_map.size() + gds_cut_layer_map.size()) + 2;

  std::vector<GPLYPLayer> lyp_layer_list;
  for (irt_int gds_layer_idx = 0; gds_layer_idx < gds_layer_size; gds_layer_idx++) {
    std::string color = color_list[gds_layer_idx % 2];
    std::string pattern = pattern_list[gds_layer_idx % 2];

    if (gds_layer_idx == 0) {
      lyp_layer_list.emplace_back(color, pattern, "DIE", gds_layer_idx, 0);
    } else if (gds_layer_idx == (gds_layer_size - 1)) {
      lyp_layer_list.emplace_back(color, pattern, "GCELL", gds_layer_idx, 0);
      lyp_layer_list.emplace_back(color, pattern, "GCELL_text", gds_layer_idx, 1);
    } else {
      if (RTUtil::exist(gds_routing_layer_map, gds_layer_idx)) {
        // routing
        std::string routing_layer_name = routing_layer_list[gds_routing_layer_map[gds_layer_idx]].get_layer_name();
        for (GPDataType routing_data_type : routing_data_type_list) {
          lyp_layer_list.emplace_back(color, pattern, RTUtil::getString(routing_layer_name, "_", GetGPDataTypeName()(routing_data_type)),
                                      gds_layer_idx, static_cast<irt_int>(routing_data_type));
        }
      } else if (RTUtil::exist(gds_cut_layer_map, gds_layer_idx)) {
        // cut
        std::string cut_layer_name = cut_layer_list[gds_cut_layer_map[gds_layer_idx]].get_layer_name();
        for (GPDataType cut_data_type : cut_data_type_list) {
          lyp_layer_list.emplace_back(color, pattern, RTUtil::getString(cut_layer_name, "_", GetGPDataTypeName()(cut_data_type)),
                                      gds_layer_idx, static_cast<irt_int>(cut_data_type));
        }
      }
    }
  }
  std::ofstream* lyp_file = RTUtil::getOutputFileStream(_gp_config.temp_directory_path + "gds.lyp");
  RTUtil::pushStream(lyp_file, "<?xml version=\"1.0\" encoding=\"utf-8\"?>", "\n");
  RTUtil::pushStream(lyp_file, "<layer-properties>", "\n");

  for (size_t i = 0; i < lyp_layer_list.size(); i++) {
    GPLYPLayer& lyp_layer = lyp_layer_list[i];
    RTUtil::pushStream(lyp_file, "<properties>", "\n");
    RTUtil::pushStream(lyp_file, "<frame-color>", lyp_layer.get_color(), "</frame-color>", "\n");
    RTUtil::pushStream(lyp_file, "<fill-color>", lyp_layer.get_color(), "</fill-color>", "\n");
    RTUtil::pushStream(lyp_file, "<frame-brightness>0</frame-brightness>", "\n");
    RTUtil::pushStream(lyp_file, "<fill-brightness>0</fill-brightness>", "\n");
    RTUtil::pushStream(lyp_file, "<dither-pattern>", lyp_layer.get_pattern(), "</dither-pattern>", "\n");
    RTUtil::pushStream(lyp_file, "<line-style/>", "\n");
    RTUtil::pushStream(lyp_file, "<valid>true</valid>", "\n");
    RTUtil::pushStream(lyp_file, "<visible>true</visible>", "\n");
    RTUtil::pushStream(lyp_file, "<transparent>false</transparent>", "\n");
    RTUtil::pushStream(lyp_file, "<width/>", "\n");
    RTUtil::pushStream(lyp_file, "<marked>false</marked>", "\n");
    RTUtil::pushStream(lyp_file, "<xfill>false</xfill>", "\n");
    RTUtil::pushStream(lyp_file, "<animation>0</animation>", "\n");
    RTUtil::pushStream(lyp_file, "<name>", lyp_layer.get_layer_name(), " ", lyp_layer.get_layer_idx(), "/", lyp_layer.get_data_type(),
                       "</name>", "\n");
    RTUtil::pushStream(lyp_file, "<source>", lyp_layer.get_layer_idx(), "/", lyp_layer.get_data_type(), "@1</source>", "\n");
    RTUtil::pushStream(lyp_file, "</properties>", "\n");
  }
  RTUtil::pushStream(lyp_file, "</layer-properties>", "\n");
  RTUtil::closeFileStream(lyp_file);
}

}  // namespace irt
