#pragma once
/**
 * @File Name: eval_io.h
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-17
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <vector>

namespace eval {
class WLNet;
class TimingNet;
}  // namespace eval

namespace iplf {

class EvalIO
{
 public:
  EvalIO() {}
  ~EvalIO() = default;

  /// io
  int64_t evalTotalWL(const std::vector<eval::WLNet*>& net_list, const std::string& wl_type);
  void estimateDelay(std::vector<eval::TimingNet*> timing_net_list, const char* sta_workspace_path, const char* sdc_file_path,
                     std::vector<const char*> lib_file_path_list);

 private:
};

}  // namespace iplf
