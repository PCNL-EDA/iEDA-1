#pragma once

#include "Config.hpp"
#include "Database.hpp"
#include "EGRConfig.hpp"
#include "EGRDatabase.hpp"
#include "builder.h"
#include "def_service.h"
#include "lef_service.h"

namespace irt {

class EGRDataManager
{
 public:
  EGRDataManager() = default;
  EGRDataManager(const EGRDataManager& other) = delete;
  EGRDataManager(EGRDataManager&& other) = delete;
  ~EGRDataManager() = default;
  EGRDataManager& operator=(const EGRDataManager& other) = delete;
  EGRDataManager& operator=(EGRDataManager&& other) = delete;
  // function
  void input(std::map<std::string, std::any>& config_map, idb::IdbBuilder* idb_builder);
  EGRConfig& getConfig() { return _egr_config; }
  EGRDatabase& getDatabase() { return _egr_database; }

 private:
  EGRConfig _egr_config;
  EGRDatabase _egr_database;
  // Helper
  std::map<irt_int, irt_int> _db_to_egr_routing_layer_idx_map;
  std::map<std::string, irt_int> _routing_layer_name_idx_map;
  std::string _design_name;

  // function
  void wrapConfig(std::map<std::string, std::any>& config_map);
  void wrapDatabase(idb::IdbBuilder* idb_builder);
  void wrapRoutingLayerList(idb::IdbBuilder* idb_builder);
  void wrapTrackAxis(RoutingLayer& routing_layer, idb::IdbLayerRouting* idb_layer);
  void wrapDie(idb::IdbBuilder* idb_builder);
  void wrapBlockageList(idb::IdbBuilder* idb_builder);
  void wrapArtificialBlockage(idb::IdbBuilder* idb_builder);
  void wrapInstanceBlockage(idb::IdbBuilder* idb_builder);
  void wrapSpecialNetBlockage(idb::IdbBuilder* idb_builder);
  void wrapNetList(idb::IdbBuilder* idb_builder);
  bool checkSkipping(idb::IdbNet* idb_net);
  void wrapPinList(EGRNet& egr_net, idb::IdbNet* idb_net);
  void wrapPinShapeList(EGRPin& egr_pin, idb::IdbPin* idb_pin);
  void wrapDrivingPin(EGRNet& egr_net, idb::IdbNet* idb_net);
  void updateHelper(idb::IdbBuilder* idb_builder);
  void buildConfig();
  void buildSkipNetNameSet();
  void buildCellSize();
  void buildBottomTopLayerIdx();
  void buildEGRStrategy();
  void buildDatabase();
  void buildRoutingLayerList();
  void buildDie();
  void buildBlockageList();
  void buildNetList();
  void buildPinList(EGRNet& egr_net);
  void buildDrivingPin(EGRNet& egr_net);
  void buildLayerResourceMap();
  void initLayerResourceMapSize();
  void addResourceMapSupply();
  void addResourceMapDemand();
  void legalizeResourceMapDemand();
  void buildHVLayerIdxList();
  Direction getRTDirectionByDB(idb::IdbLayerDirection idb_direction);
  irt_int getEGRLayerIndexByDB(irt_int db_layer_idx);
  PlanarRect getGridRect(PlanarRect& real_rect);
  void printConfig();
  void printDatabase();
};

}  // namespace irt
