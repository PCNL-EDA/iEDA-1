#pragma once

#include <string>

#include "../../../database/interaction/ids.hpp"
#include "DRC.h"
#include "DrcEnum.h"

namespace idrc {

class DrcConfig;
class DrcDesign;
class Tech;
class RoutingSpacingCheck;
class CutSpacingCheck;
class RoutingWidthCheck;
class EnclosedAreaCheck;
class RoutingAreaCheck;
class EnclosureCheck;
class DrcSpot;
class DrcRect;
class DrcPolygon;
class DrcPoly;
class RegionQuery;
class IDRWrapper;
class DrcIDBWrapper;
class DrcNet;
class SpotParser;
class MultiPatterning;
class DrcConflictGraph;
class EOLSpacingCheck;

#define DrcAPIInst DrcAPI::getInst()
class DrcAPI
{
 public:
  static DrcAPI& getInst()
  {
    if (_drc_api_instance == nullptr) {
      _drc_api_instance = new DrcAPI();
    }
    return *_drc_api_instance;
  }

  static void destroyInst();
  // interact function
  RegionQuery* init();
  void runDrc();
  bool check(std::vector<ids::DRCTask> task_list);
  void add(std::vector<ids::DRCTask> task_list);
  void del(std::vector<ids::DRCTask> task_list);
  std::map<std::string, int> getCheckResult(RegionQuery* region_query);

  std::map<std::string, int> getCheckResult();
  std::map<std::string, std::vector<DrcViolationSpot*>> getDetailCheckResult();

  // Get the maximum influence region of spacing (Common,EOL,Corner_fill)
  std::vector<DrcRect*> getMaxScope(std::vector<DrcRect*> origin_rect_list);
  // Get the minimum influence region of spacing (Common,EOL,Corner_fill)
  std::vector<DrcRect*> getMinScope(std::vector<DrcRect*> origin_rect_list);
  DrcRect* getDrcRect(int net_id, int lb_x, int lb_y, int rt_x, int rt_y, std::string layer_name, bool is_artificial = false);

  //
  void initDRC();
  // RegionQuery* createRTree(ids::DRCEnv env);
  bool checkDRC(std::vector<DrcRect*> origin_rect_list);

 private:
  static DrcAPI* _drc_api_instance;
  Tech* _tech;
  DrcIDBWrapper* _idb_wrapper = nullptr;

  DRC* _drc = nullptr;

  DrcAPI() = default;
  ~DrcAPI() = default;
  void getCommonSpacingMinRegion(DrcRect* common_spacing_min_region, DrcRect* drc_rect);
  void getCommonSpacingMaxRegion(DrcRect* common_spacing_min_region, DrcRect* drc_rect);
  // void initSpacingRegion(ids::DRCEnv env, RegionQuery* region_query);
  void addEOLSpacingRegion(DrcPoly* drc_poly, RegionQuery* region_query);
  void addCommonSpacingRegion(DrcRect* drc_rect, RegionQuery* region_query);

  void initRegionQuery(std::vector<DrcRect*> origin_rect_list, RegionQuery* region_query);
  // DrcRect* getDrcRect(ids::DRCRect ids_rect);
  RTreeBox getRTreeBox(DrcRect* rect);
  void initPoly(std::map<int, DrcNet>& nets, RegionQuery* region_query);
  void initPolyPolygon(DrcNet* net);
  void initPolyInnerEdges(DrcNet* net, DrcPoly* poly, const BoostPolygon& hole_poly, int layer_id, RegionQuery* region_query);
  void initPolyOuterEdges(DrcNet* net, DrcPoly* poly, DrcPolygon* polygon, int layer_id, RegionQuery* region_query);
  void initPolyEdges(DrcNet* net, RegionQuery* region_query);
  void initNets(std::vector<DrcRect*>& drc_rect_list, std::map<int, DrcNet>& nets);

  // scope
  void getCommonSpacingScope(std::vector<DrcRect*>& max_scope_list, std::map<int, DrcNet>& nets, bool is_max);
  void getCommonSpacingScopeRect(DrcRect* target_rect, DrcRect* scope_rect, int spacing);
  void getEOLSpacingScope(std::vector<DrcRect*>& max_scope_list, std::map<int, DrcNet>& target_nets, bool is_max);
  void getCornerFillSpacingScope(std::vector<DrcRect*>& max_scope_list, std::map<int, DrcNet>& target_nets);

  // check
  bool checkSpacing(RegionQuery* region_query, DrcRect* check_rect);
  bool checkShape(RegionQuery* region_query, DrcRect* drc_rect);

  void queryInMaxScope(DrcRect* check_rect, std::map<void*, std::set<ScopeType>>& max_scope_query_result);
  bool checkSpacing_rect(DrcRect* check_rect, RegionQuery* region_query);
  void mergeRectToPoly(DrcRect* check_rect, RegionQuery* region_query, DrcPoly* poly);
};

}  // namespace idrc
