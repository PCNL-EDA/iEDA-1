/**
 * @file CmdGetPorts.cc
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief
 * @version 0.1
 * @date 2022-02-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Cmd.hh"
#include "log/Log.hh"
#include "sdc/SdcCollection.hh"

namespace ista {

CmdGetPorts::CmdGetPorts(const char* cmd_name) : TclCmd(cmd_name) {
  auto* clock_list_arg = new TclStringListOption("patterns", 1, {});
  addOption(clock_list_arg);
}

unsigned CmdGetPorts::check() { return 1; }

/**
 * @brief execute the nl.
 *
 * @return unsigned
 */
unsigned CmdGetPorts::exec() {
  Sta* ista = Sta::getOrCreateSta();
  auto& the_constrain = ista->get_constrains();

  auto* port_list_arg = getOptionOrArg("patterns");
  auto port_list = port_list_arg->getStringList();

  std::vector<SdcCollectionObj> obj_list;

  Netlist* design_nl = ista->get_netlist();
  for (auto& port_name : port_list) {
    auto ports = design_nl->findPort(port_name.c_str(), false, false);
    LOG_FATAL_IF(ports.empty())
        << "get_ports " << port_name << " was not found.";
    for (auto* design_obj : ports) {
      obj_list.emplace_back(design_obj);
    }
  }

  auto* sdc_collection = new SdcCollection(SdcCollection::CollectionType::kPin,
                                           std::move(obj_list));

  the_constrain->addSdcCollection(sdc_collection);

  char* result = TclEncodeResult::encode(sdc_collection);
  ScriptEngine::getOrCreateInstance()->setResult(result);

  return 1;
}
}  // namespace ista
