#include "RTAPI.hpp"
#include "tcl_rt.h"
#include "tcl_util.h"

namespace tcl {

// public

TclInitRT::TclInitRT(const char* cmd_name) : TclCmd(cmd_name)
{
  // std::string output_def_file_path;  // required
  _config_list.push_back(std::make_pair("-output_def_file_path", ValueType::kString));
  // std::string temp_directory_path;  // required
  _config_list.push_back(std::make_pair("-temp_directory_path", ValueType::kString));
  // irt_int log_level;  // optional
  _config_list.push_back(std::make_pair("-log_level", ValueType::kInt));
  // irt_int thread_number;  // optional
  _config_list.push_back(std::make_pair("-thread_number", ValueType::kInt));
  // std::string bottom_routing_layer;  // optional
  _config_list.push_back(std::make_pair("-bottom_routing_layer", ValueType::kString));
  // std::string top_routing_layer;     // optional
  _config_list.push_back(std::make_pair("-top_routing_layer", ValueType::kString));
  // std::map<std::string, double> layer_utilization_ratio;  // optional
  _config_list.push_back(std::make_pair("-layer_utilization_ratio", ValueType::kStringDoubleMap));
  // irt_int enable_output_gds_files;  // optional
  _config_list.push_back(std::make_pair("-enable_output_gds_files", ValueType::kInt));
  // double resource_allocate_initial_penalty;               // optional
  _config_list.push_back(std::make_pair("-resource_allocate_initial_penalty", ValueType::kDouble));
  // double resource_allocate_penalty_drop_rate;             // optional
  _config_list.push_back(std::make_pair("-resource_allocate_penalty_drop_rate", ValueType::kDouble));
  // irt_int resource_allocate_outer_iter_num;               // optional
  _config_list.push_back(std::make_pair("-resource_allocate_outer_iter_num", ValueType::kInt));
  // irt_int resource_allocate_inner_iter_num;               // optional
  _config_list.push_back(std::make_pair("-resource_allocate_inner_iter_num", ValueType::kInt));

  TclUtil::addOption(this, _config_list);
}

unsigned TclInitRT::exec()
{
  std::map<std::string, std::any> config_map = TclUtil::getConfigMap(this, _config_list);
  RTAPI_INST.initRT(config_map);
  return 1;
}

// private

}  // namespace tcl
