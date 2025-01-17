#pragma once

#include "RTU.hpp"
#include "Stage.hpp"

namespace irt {

class Config
{
 public:
  Config() = default;
  ~Config() = default;
  /////////////////////////////////////////////
  // **********        RT         ********** //
  std::string output_def_file_path;                       // required
  std::string temp_directory_path;                        // required
  irt_int log_level;                                      // optional
  irt_int thread_number;                                  // optional
  std::string bottom_routing_layer;                       // optional
  std::string top_routing_layer;                          // optional
  std::map<std::string, double> layer_utilization_ratio;  // optional
  irt_int enable_output_gds_files;                        // optional
  double resource_allocate_initial_penalty;               // optional
  double resource_allocate_penalty_drop_rate;             // optional
  irt_int resource_allocate_outer_iter_num;               // optional
  irt_int resource_allocate_inner_iter_num;               // optional
  /////////////////////////////////////////////
  // **********        RT         ********** //
  std::string log_file_path;                              // building
  irt_int bottom_routing_layer_idx;                       // building
  irt_int top_routing_layer_idx;                          // building
  std::map<irt_int, double> layer_idx_utilization_ratio;  // building
  // **********    DataManager    ********** //
  std::string dm_temp_directory_path;  // building
  // **********  DetailedRouter   ********** //
  std::string dr_temp_directory_path;  // building
  // **********    GDSPlotter     ********** //
  std::string gp_temp_directory_path;  // building
  // **********   GlobalRouter    ********** //
  std::string gr_temp_directory_path;  // building
  // **********   PinAccessor     ********** //
  std::string pa_temp_directory_path;  // building
  // ********   ResourceAllocator   ******** //
  std::string ra_temp_directory_path;  // building
  // **********   TrackAssigner   ********** //
  std::string ta_temp_directory_path;  // building
  // **********  UniversalRouter  ********** //
  std::string ur_temp_directory_path;  // building
  // ********** ViolationRepairer ********** //
  std::string vr_temp_directory_path;  // building
  /////////////////////////////////////////////
};

}  // namespace irt
