#pragma once

#include <map>
#include <string>
#include <vector>
namespace ieda {
class Log;
class Str;
class Time;
}  // namespace ieda
namespace icts {
class TimingNode;
class CtsCellLib;
class CtsLibs;
class CtsReportTable;
class CtsLog;
class CtsInstance;
class CtsConfig;
class CtsDesign;
class CtsDBWrapper;
class CtsPin;
class CtsSignalWire;
class CtsNet;
class EvalNet;
class Evaluator;
class Synthesis;
class Balancer;
class OptiNet;
class Endpoint;
enum class FitType;
class ModelBase;
class ModelFactory;
class MplHelper;
template <typename T>
class CtsPoint;
using Point = icts::CtsPoint<int>;
template <typename T>
class CtsSegment;
using Segment = icts::CtsSegment<int>;
template <typename T>
class CtsPolygon;
using Polygon = icts::CtsPolygon<int>;
template <typename T>
struct DataTraits;
}  // namespace icts

namespace ista {
class TimingEngine;
class TimingIDBAdapter;
class DesignObject;
class RctNode;
class Net;
}  // namespace ista
namespace idb {
class IdbPin;
class IdbInstance;
class IdbNet;
}  // namespace idb

namespace ito {
class Tree;
}  // namespace ito
namespace idrc {

class RegionQuery;
class DrcRect;

}  // namespace idrc

namespace irt {

enum class Stage;
class PlanarRect;
class LayerRect;

template <typename T>
class Segment;

template <typename T>
class MTree;

class Net;
class AccessPoint;
class GRNode;

}  // namespace irt

namespace eval {

class TimingNet;
class TimingPin;
class TileGrid;

}  // namespace eval

namespace ids {

struct Segment {
  int first_x;
  int first_y;
  std::string first_layer_name;
  int second_x;
  int second_y;
  std::string second_layer_name;
};

enum class PHYNodeType { kNone = 0, kWire = 1, kVia = 2 };

struct Wire {
  int first_x;
  int first_y;
  int second_x;
  int second_y;
  std::string layer_name;
};

struct Via {
  std::string via_name;
  int x;
  int y;
};

struct PHYNode {
  PHYNodeType type;
  Wire wire;
  Via via;
};

enum class AccessPointType {
  kNone = 0,
  kPrefTrackGrid = 1,
  kCurrTrackGrid = 2,
  kCurrTrackCenter = 3,
  kCurrShapeCenter = 4
};

struct AccessPoint {
  int x;
  int y;
  std::string layer_name;
  AccessPointType type;
  std::vector<std::string> via_name_list;
};

struct CellMaster {
  std::string cell_master_name;
  std::map<std::string, std::vector<ids::AccessPoint>> pin_name_pa_list;
};

struct DRCTask {
  idrc::RegionQuery* region_query;
  std::vector<idrc::DrcRect*> drc_rect_list;
};

struct DRCRect {
  int32_t so_id = -1;  // Distinguish self and others

  int32_t lb_x = -1;
  int32_t lb_y = -1;

  int32_t rt_x = -1;
  int32_t rt_y = -1;

  std::string layer_name;

  bool is_artificial = false;
};

}  // namespace ids
