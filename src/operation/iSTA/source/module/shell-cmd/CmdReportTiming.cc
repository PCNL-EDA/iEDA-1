/**
 * @file CmdReportTiming.cc
 * @author Wang Hao (harry0789@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-10-12
 */
#include "ShellCmd.hh"
#include "sta/Sta.hh"

namespace ista {
CmdReportTiming::CmdReportTiming(const char* cmd_name) : TclCmd(cmd_name) {
  auto* digits_option = new TclIntOption("-digits", 0, 0);
  addOption(digits_option);
  auto* path_delay_option = new TclStringOption("-delay_type", 0, nullptr);
  addOption(path_delay_option);
  auto* exclude_cell_names_option =
      new TclStringListOption("-exclude_cell_names", 0, {});
  addOption(exclude_cell_names_option);
  auto* derate_option = new TclSwitchOption("-derate");
  addOption(derate_option);
  auto* is_clock_cap_option = new TclSwitchOption("-is_clock_cap");
  addOption(is_clock_cap_option);
}

unsigned CmdReportTiming::check() { return 1; }

unsigned CmdReportTiming::exec() {
  if (!check()) {
    return 0;
  }

  TclOption* exclude_cell_names_option = getOptionOrArg("-exclude_cell_names");
  std::set<std::string> new_exclude_cell_names;
  if (exclude_cell_names_option) {
    auto exclude_cell_names = exclude_cell_names_option->getStringList();
    for (const auto& exclude_cell_name : exclude_cell_names) {
      new_exclude_cell_names.insert(exclude_cell_name);
    }
  }

  TclOption* derate_option = getOptionOrArg("-derate");
  bool is_derate = false;
  if (derate_option) {
    is_derate = true;
  }

  TclOption* is_clock_cap_option = getOptionOrArg("-is_clock_cap");
  bool is_clock_cap = false;
  if (is_clock_cap_option) {
    is_clock_cap = true;
  }

  Sta* ista = Sta::getOrCreateSta();
  ista->buildGraph();
  ista->updateTiming();
  ista->reportTiming(std::move(new_exclude_cell_names), is_derate,
                     is_clock_cap);

  return 1;
}

}  // namespace ista