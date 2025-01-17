#pragma once

#include "Config.hpp"
#include "Database.hpp"
#include "GridMap.hpp"
#include "RAConfig.hpp"
#include "RADataManager.hpp"
#include "RADatabase.hpp"
#include "RAGCell.hpp"
#include "RAModel.hpp"
#include "RANet.hpp"

namespace irt {

#define RA_INST (irt::ResourceAllocator::getInst())

class ResourceAllocator
{
 public:
  static void initInst(Config& config, Database& database);
  static ResourceAllocator& getInst();
  static void destroyInst();
  // function
  void allocate(std::vector<Net>& net_list);

 private:
  // self
  static ResourceAllocator* _ra_instance;
  // config & database
  RADataManager _ra_data_manager;

  ResourceAllocator(Config& config, Database& database) { init(config, database); }
  ResourceAllocator(const ResourceAllocator& other) = delete;
  ResourceAllocator(ResourceAllocator&& other) = delete;
  ~ResourceAllocator() = default;
  ResourceAllocator& operator=(const ResourceAllocator& other) = delete;
  ResourceAllocator& operator=(ResourceAllocator&& other) = delete;
  // function
  void init(Config& config, Database& database);
  void allocateRANetList(std::vector<RANet>& ra_net_list);

#if 1  // build ra_model
  RAModel initRAModel(std::vector<RANet>& ra_net_list);
  void buildRAModel(RAModel& ra_model);
  void initRANetDemand(RAModel& ra_model);
  void initRAGCellList(RAModel& ra_model);
  void addBlockageList(RAModel& ra_model);
  void buildAccessMap(RAModel& ra_model);
  void calcRAGCellSupply(RAModel& ra_model);
  std::vector<PlanarRect> getWireList(RAGCell& ra_gcell, RoutingLayer& routing_layer);
  void buildRelation(RAModel& ra_model);
  void initTempObject(RAModel& ra_model);
#endif

#if 1  // check ra_model
  void checkRAModel(RAModel& ra_model);
#endif

#if 1  // allocate ra_model
  void allocateRAModel(RAModel& ra_model);
  void calcNablaF(RAModel& ra_model, double penalty_para);
  double calcAlpha(RAModel& ra_model, double penalty_para);
  double updateResult(RAModel& ra_model);
#endif

#if 1  // update ra_model
  void updateRAModel(RAModel& ra_model);
  void updateAllocationMap(RAModel& ra_model);
  GridMap<double> getCostMap(GridMap<double>& allocation_map, double lower_cost);
  void normalizeCostMap(GridMap<double>& cost_map, double lower_cost);
  void updateOriginRACostMap(RAModel& ra_model);
#endif

#if 1  // report ra_model
  void reportRAModel(RAModel& ra_model);
  void countRAModel(RAModel& ra_model);
  void reportTable(RAModel& ra_model);
#endif
};

}  // namespace irt
