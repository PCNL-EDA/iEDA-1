#include <set>

#include "RTAPI.hpp"
#include "flow_config.h"
#include "tcl_rt.h"
#include "tcl_util.h"
#include "usage/usage.hh"

namespace tcl {

TclRunRT::TclRunRT(const char* cmd_name) : TclCmd(cmd_name)
{
  _config_list.push_back(std::make_pair("-flow", ValueType::kStringList));
  TclUtil::addOption(this, _config_list);
}

unsigned TclRunRT::exec()
{
  if (!check()) {
    return 0;
  }

  iplf::flowConfigInst->set_status_stage("iRT - Routing");
  ieda::Stats stats;

  std::map<std::string, std::any> config_map = TclUtil::getConfigMap(this, _config_list);

  std::vector<irt::Tool> tool_list;
  if (config_map.find("-flow") == config_map.end()) {
    tool_list.push_back(irt::Tool::kPinAccessor);
    tool_list.push_back(irt::Tool::kResourceAllocator);
    tool_list.push_back(irt::Tool::kGlobalRouter);
    tool_list.push_back(irt::Tool::kTrackAssigner);
    tool_list.push_back(irt::Tool::kDetailedRouter);
    tool_list.push_back(irt::Tool::kViolationRepairer);
  } else {
    for (std::string tool_name : std::any_cast<std::vector<std::string>>(config_map["-flow"])) {
      if (tool_name == "pa") {
        tool_list.push_back(irt::Tool::kPinAccessor);
      } else if (tool_name == "ra") {
        tool_list.push_back(irt::Tool::kResourceAllocator);
      } else if (tool_name == "gr") {
        tool_list.push_back(irt::Tool::kGlobalRouter);
      } else if (tool_name == "ta") {
        tool_list.push_back(irt::Tool::kTrackAssigner);
      } else if (tool_name == "dr") {
        tool_list.push_back(irt::Tool::kDetailedRouter);
      } else if (tool_name == "vr") {
        tool_list.push_back(irt::Tool::kViolationRepairer);
      }
    }
  }
  RTAPI_INST.runRT(tool_list);

  iplf::flowConfigInst->add_status_runtime(stats.elapsedRunTime());
  iplf::flowConfigInst->set_status_memmory(stats.memoryDelta());

  return 1;
}

}  // namespace tcl
