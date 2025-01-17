#pragma once

#include "Config.hpp"
#include "Database.hpp"
#include "GRConfig.hpp"
#include "GRDataManager.hpp"
#include "GRModel.hpp"
#include "RTU.hpp"
#include "SortStatus.hpp"
#include "SortType.hpp"

namespace irt {

#define GR_INST (irt::GlobalRouter::getInst())

class GlobalRouter
{
 public:
  static void initInst(Config& config, Database& database);
  static GlobalRouter& getInst();
  static void destroyInst();
  // function
  void route(std::vector<Net>& net_list);

 private:
  // self
  static GlobalRouter* _gr_instance;
  // config & database
  GRDataManager _gr_data_manager;

  GlobalRouter(Config& config, Database& database) { init(config, database); }
  GlobalRouter(const GlobalRouter& other) = delete;
  GlobalRouter(GlobalRouter&& other) = delete;
  ~GlobalRouter() = default;
  GlobalRouter& operator=(const GlobalRouter& other) = delete;
  GlobalRouter& operator=(GlobalRouter&& other) = delete;
  // function
  void init(Config& config, Database& database);
  void routeGRNetList(std::vector<GRNet>& gr_net_list);

#if 1  // build gr_model
  GRModel initGRModel(std::vector<GRNet>& gr_net_list);
  void buildGRModel(GRModel& gr_model);
  void buildNeighborMap(GRModel& gr_model);
  void buildNodeSupply(GRModel& gr_model);
  void initNodeRealRect(GRModel& gr_model);
  void addBlockageList(GRModel& gr_model);
  void addNetRegionList(GRModel& gr_model);
  void calcAreaSupply(GRModel& gr_model);
  void initSingleResource(GRNode& gr_node, RoutingLayer& routing_layer);
  void initResourceSupply(GRNode& gr_node, RoutingLayer& routing_layer);
  std::vector<PlanarRect> getWireList(GRNode& gr_node, RoutingLayer& routing_layer);
  void buildAccessMap(GRModel& gr_model);
  void buildGRNetPriority(GRModel& gr_model);
#endif

#if 1  // check gr_model
  void checkGRModel(GRModel& gr_model);
#endif

#if 1  // sort gr_model
  void sortGRModel(GRModel& gr_model);
  bool sortByMultiLevel(GRNet& net1, GRNet& net2);
  SortStatus sortByClockPriority(GRNet& net1, GRNet& net2);
  SortStatus sortByRoutingAreaASC(GRNet& net1, GRNet& net2);
  SortStatus sortByLengthWidthRatioDESC(GRNet& net1, GRNet& net2);
  SortStatus sortByPinNumDESC(GRNet& net1, GRNet& net2);
#endif

#if 1  // route gr_model
  void routeGRModel(GRModel& gr_model);
  void routeGRNet(GRModel& gr_model, GRNet& gr_net);
  void initRoutingInfo(GRModel& gr_model, GRNet& gr_net);
  bool isConnectedAllEnd(GRModel& gr_model);
  void routeSinglePath(GRModel& gr_model);
  void initPathHead(GRModel& gr_model);
  bool searchEnded(GRModel& gr_model);
  void expandSearching(GRModel& gr_model);
  bool passCheckingSegment(GRModel& gr_model, GRNode* start_node, GRNode* end_node);
  bool replaceParentNode(GRModel& gr_model, GRNode* parent_node, GRNode* child_node);
  void resetPathHead(GRModel& gr_model);
  void rerouteByEnlarging(GRModel& gr_model);
  bool isRoutingFailed(GRModel& gr_model);
  void resetSinglePath(GRModel& gr_model);
  void rerouteByforcing(GRModel& gr_model);
  void updatePathResult(GRModel& gr_model);
  void resetStartAndEnd(GRModel& gr_model);
  void updateNetResult(GRModel& gr_model, GRNet& gr_net);
  void resetSingleNet(GRModel& gr_model);
  void pushToOpenList(GRModel& gr_model, GRNode* curr_node);
  GRNode* popFromOpenList(GRModel& gr_model);
  double getKnowCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node);
  double getJointCost(GRModel& gr_model, GRNode* curr_node, Orientation orientation);
  double getEstimateCostToEnd(GRModel& gr_model, GRNode* curr_node);
  double getEstimateCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node);
  Orientation getOrientation(GRNode* start_node, GRNode* end_node);
  double getWireCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node);
  double getViaCost(GRModel& gr_model, GRNode* start_node, GRNode* end_node);
#endif

#if 1  // plot gr_model
  void plotGRModel(GRModel& gr_model, irt_int curr_net_idx = -1);
#endif

#if 1  // update gr_model
  void updateGRModel(GRModel& gr_model);
  void initRoutingResult(GRNet& gr_net);
  RTNode convertToRTNode(LayerCoord& coord, std::map<LayerCoord, std::set<irt_int>, CmpLayerCoordByXASC>& key_coord_pin_map);
  void buildRoutingResult(GRNet& gr_net);
  void buildDRNode(TNode<RTNode>* parent_node, TNode<RTNode>* child_node);
  void buildTANode(TNode<RTNode>* parent_node, TNode<RTNode>* child_node);
  void updateOriginGRResultTree(GRModel& gr_model);
#endif

#if 1  // report gr_model
  void reportGRModel(GRModel& gr_model);
  void countGRModel(GRModel& gr_model);
  void reportTable(GRModel& gr_model);
#endif
};

}  // namespace irt
