/**
 * @file CmdSetPropagatedClock.cc
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The sdc set_propagated_clock implemention.
 * @version 0.1
 * @date 2022-02-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Cmd.hh"
#include "sdc/SdcClock.hh"

namespace ista {

CmdSetPropagatedClock::CmdSetPropagatedClock(const char* cmd_name)
    : TclCmd(cmd_name) {
  auto* object_list_arg = new TclStringOption("object_list", 1, {});
  addOption(object_list_arg);
}

unsigned CmdSetPropagatedClock::check() { return 1; }

unsigned CmdSetPropagatedClock::exec() {
  Sta* ista = Sta::getOrCreateSta();
  Netlist* design_nl = ista->get_netlist();

  TclOption* object_list_option = getOptionOrArg("object_list");
  auto* object_list_str = object_list_option->getStringVal();

  auto object_list = FindObjOfSdc(object_list_str, design_nl);
  LOG_FATAL_IF(object_list.empty()) << "object list is empty.";

  for (auto& object : object_list) {
    std::visit(
        overloaded{
            [](SdcCommandObj* sdc_obj) {
              if (sdc_obj->isAllClock()) {
                auto* all_clocks = dynamic_cast<SdcAllClocks*>(sdc_obj);
                auto& sdc_clocks = all_clocks->get_clocks();
                for (auto* sdc_clock : sdc_clocks) {
                  sdc_clock->set_is_propagated();
                  LOG_INFO << sdc_clock->get_clock_name() << " is propagated.";
                }
              }
            },
            [](DesignObject* design_obj) {
              LOG_FATAL << "set_clock_latency not support design object yet.";
            },
        },
        object);
  }

  DLOG_INFO << "exec set_propagated_clock cmd.";

  return 1;
}

}  // namespace ista
