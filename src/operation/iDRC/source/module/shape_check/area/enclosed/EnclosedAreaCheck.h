#pragma once

#include <set>

#include "BoostType.h"
#include "RegionQuery.h"

namespace idrc {

class DrcConfig;
class Tech;
class DrcSpot;

class DrcNet;

enum class ViolationType;

template <typename T>
class DrcRectangle;

class EnclosedAreaCheck
{
 public:
  // static EnclosedAreaCheck* getInstance(DrcConfig* config = nullptr, Tech* tech = nullptr)
  // {
  //   static EnclosedAreaCheck instance(config, tech);
  //   return &instance;
  // }
  EnclosedAreaCheck(DrcConfig* config, Tech* tech) { init(config, tech); }
  EnclosedAreaCheck(const EnclosedAreaCheck& other) = delete;
  EnclosedAreaCheck(EnclosedAreaCheck&& other) = delete;
  EnclosedAreaCheck(Tech* tech, RegionQuery* region_query) : _tech(tech), _region_query(region_query) {}
  ~EnclosedAreaCheck() {}
  EnclosedAreaCheck& operator=(const EnclosedAreaCheck& other) = delete;
  EnclosedAreaCheck& operator=(EnclosedAreaCheck&& other) = delete;

  // setter
  // getter
  std::map<int, std::vector<DrcSpot>>& get_routing_layer_to_spots_map() { return _routing_layer_to_spots_map; }
  // function
  void checkEnclosedArea(DrcNet* target_net);
  void checkEnclosedArea(const LayerNameToRTreeMap& layer_to_rects_tree_map);

  void reset();

  int get_enclosed_area_violation_num();

 private:
  DrcConfig* _config;
  Tech* _tech;
  RegionQuery* _region_query;
  std::map<int, PolygonSet> _layer_to_polygons_map;
  std::map<int, std::vector<DrcSpot>> _routing_layer_to_spots_map;

  void init(DrcConfig* config, Tech* tech);

  void initLayerPolygonSet(DrcNet* target_net);
  void initLayerToPolygonSetFromRoutingRects(DrcNet* target_net);
  void initLayerToPolygonSetFromPinRects(DrcNet* target_net);
  void checkEnclosedArea();
  void checkEnclosedAreaFromHolePlygonList(const std::vector<PolygonWithHoles>& hole_polygon_list, int requre_enclosed_area, int layerId);
  void add_spot(int layerId, const DrcRectangle<int>& vialation_box, ViolationType type);

  ////**********************interact with iRT********************////
  void initLayerPolygonSet(const LayerNameToRTreeMap& layer_to_rects_tree_map);
};  // namespace idrc

}  // namespace idrc
