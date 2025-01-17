#pragma once

#include "Logger.hpp"
#include "RTU.hpp"
#include "RTUtil.hpp"

namespace irt {

class Helper
{
 public:
  Helper() = default;
  ~Helper() = default;
  // getter
  std::string& get_design_name() { return _design_name; }
  std::vector<std::string>& get_lef_file_path_list() { return _lef_file_path_list; }
  std::string& get_def_file_path() { return _def_file_path; }
  std::map<irt_int, irt_int>& get_routing_layer_idx_db_to_rt_map() { return _routing_layer_idx_db_to_rt_map; }
  std::map<std::string, irt_int>& get_routing_layer_name_to_idx_map() { return _routing_layer_name_to_idx_map; }
  std::map<irt_int, irt_int>& get_cut_layer_idx_db_to_rt_map() { return _cut_layer_idx_db_to_rt_map; }
  std::map<std::string, irt_int>& get_cut_layer_name_to_idx_map() { return _cut_layer_name_to_idx_map; }
  std::map<std::string, std::pair<irt_int, irt_int>>& get_via_name_to_idx_map() { return _via_name_to_idx_map; }
  // setter
  void set_design_name(const std::string& design_name) { _design_name = design_name; }
  void set_lef_file_path_list(const std::vector<std::string>& lef_file_path_list) { _lef_file_path_list = lef_file_path_list; }
  void set_def_file_path(const std::string& def_file_path) { _def_file_path = def_file_path; }
  // function
  inline irt_int wrapIDBRoutingLayerIdxToRT(const irt_int idb_layer_id);
  inline irt_int wrapIDBCutLayerIdxToRT(const irt_int idb_layer_id);
  inline irt_int getRoutingLayerIdxByName(const std::string& routing_layer_name);
  inline irt_int getCutLayerIdxByName(const std::string& cut_layer_name);
  inline std::pair<irt_int, irt_int> getRTViaPairIdxByName(const std::string& via_name);

 private:
  std::string _design_name;
  std::vector<std::string> _lef_file_path_list;
  std::string _def_file_path;
  std::map<irt_int, irt_int> _routing_layer_idx_db_to_rt_map;
  std::map<std::string, irt_int> _routing_layer_name_to_idx_map;
  std::map<irt_int, irt_int> _cut_layer_idx_db_to_rt_map;
  std::map<std::string, irt_int> _cut_layer_name_to_idx_map;
  std::map<std::string, std::pair<irt_int, irt_int>> _via_name_to_idx_map;
};

inline irt_int Helper::wrapIDBRoutingLayerIdxToRT(const irt_int idb_layer_id)
{
  irt_int routing_layer_idx = -1;
  if (RTUtil::exist(_routing_layer_idx_db_to_rt_map, idb_layer_id)) {
    routing_layer_idx = _routing_layer_idx_db_to_rt_map[idb_layer_id];
  } else {
    LOG_INST.error(Loc::current(), "The idb_layer_id ", idb_layer_id, " is not exist!");
  }
  return routing_layer_idx;
}

inline irt_int Helper::getRoutingLayerIdxByName(const std::string& routing_layer_name)
{
  irt_int routing_layer_idx = -1;
  if (RTUtil::exist(_routing_layer_name_to_idx_map, routing_layer_name)) {
    routing_layer_idx = _routing_layer_name_to_idx_map[routing_layer_name];
  } else {
    LOG_INST.error(Loc::current(), "The routing_layer_name '", routing_layer_name, "' is not exist!");
  }
  return routing_layer_idx;
}

inline irt_int Helper::wrapIDBCutLayerIdxToRT(const irt_int idb_layer_id)
{
  irt_int cut_layer_idx = -1;
  if (RTUtil::exist(_cut_layer_idx_db_to_rt_map, idb_layer_id)) {
    cut_layer_idx = _cut_layer_idx_db_to_rt_map[idb_layer_id];
  } else {
    LOG_INST.error(Loc::current(), "The idb_layer_id ", idb_layer_id, " is not exist!");
  }
  return cut_layer_idx;
}

inline irt_int Helper::getCutLayerIdxByName(const std::string& cut_layer_name)
{
  irt_int cut_layer_idx = -1;
  if (RTUtil::exist(_cut_layer_name_to_idx_map, cut_layer_name)) {
    cut_layer_idx = _cut_layer_name_to_idx_map[cut_layer_name];
  } else {
    LOG_INST.error(Loc::current(), "The cut_layer_name ", cut_layer_name, " is not exist!");
  }
  return cut_layer_idx;
}

inline std::pair<irt_int, irt_int> Helper::getRTViaPairIdxByName(const std::string& via_name)
{
  std::pair<irt_int, irt_int> via_idx;
  if (RTUtil::exist(_via_name_to_idx_map, via_name)) {
    via_idx = _via_name_to_idx_map[via_name];
  } else {
    LOG_INST.error(Loc::current(), "The via_name ", via_name, " is not exist!");
  }
  return via_idx;
}

}  // namespace irt
