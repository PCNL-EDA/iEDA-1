#ifndef SRC_EVALUATOR_SOURCE_WRAPPER_CONFIG_TIMINGCONFIG_HPP_
#define SRC_EVALUATOR_SOURCE_WRAPPER_CONFIG_TIMINGCONFIG_HPP_

#include <string>
#include <vector>

namespace eval {

class TimingConfig
{
 public:
  TimingConfig() = default;
  ~TimingConfig() = default;

  // getter
  bool enable_eval() const { return _enable_eval; }
  std::string get_sta_workspace() const { return _sta_workspace; }
  std::string get_sdc_file() const { return _sdc_file; }
  std::vector<std::string> get_lib_file_list() const { return _lib_file_list; }
  std::string get_output_dir() const { return _output_dir; }

  // setter
  void enable_eval(const bool& enable_eval) { _enable_eval = enable_eval; }
  void set_sta_workspace(const std::string& sta_workspace) { _sta_workspace = sta_workspace; }
  void set_sdc_file(const std::string& sdc_file) { _sdc_file = sdc_file; }
  void set_lib_file_list(const std::vector<std::string>& lib_file_list) { _lib_file_list = lib_file_list; }
  void set_output_dir(const std::string& output_dir) { _output_dir = output_dir; }

 private:
  bool _enable_eval;
  std::string _sta_workspace;
  std::string _sdc_file;
  std::vector<std::string> _lib_file_list;
  std::string _output_dir;
};
}  // namespace eval

#endif  // SRC_EVALUATOR_SOURCE_WRAPPER_CONFIG_TIMINGCONFIG_HPP_
