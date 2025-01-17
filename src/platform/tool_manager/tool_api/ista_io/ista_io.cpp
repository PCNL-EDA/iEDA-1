#include "ista_io.h"

#include <filesystem>

#include "IdbEnum.h"
#include "TimingEngine.hh"
#include "TimingIDBAdapter.hh"
#include "builder.h"
#include "flow_config.h"
#include "idm.h"

namespace iplf {
StaIO* StaIO::_instance = nullptr;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @Brief : run sta
 * @param  path path is not a must, if path is empty, using the DB Config output
 * path
 * @return true
 * @return false
 */
bool StaIO::autoRunSTA(std::string path)
{
  flowConfigInst->set_status_stage("iSTA - Static Timing Analysis");

  ieda::Stats stats;

  /// init
  setStaWorkDirectory(path);
  std::vector<std::string> paths;
  runLiberty(paths);
  readIdb();
  runSDC();
  /// run
  reportTiming();

  flowConfigInst->add_status_runtime(stats.elapsedRunTime());
  flowConfigInst->set_status_memmory(stats.memoryDelta());

  return true;
}

bool StaIO::buildClockTree(std::string sta_path)
{
  /// build clock tree by sta
  setStaWorkDirectory(sta_path);

  /// init
  std::vector<std::string> paths;
  runLiberty(paths);

  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();

  timing_engine->get_ista()->set_analysis_mode(AnalysisMode::kMax);
  timing_engine->get_ista()->set_n_worst_path_per_clock(1);
  timing_engine->get_ista()->set_top_module_name("asic_top");

  readIdb();

  runSDC();
  timing_engine->buildGraph();

  runSpef();

  timing_engine->updateTiming();
  timing_engine->buildClockTrees();
  return true;
}

std::vector<std::unique_ptr<ista::StaClockTree>>& StaIO::getClockTree()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  return timing_engine->getClockTrees();
}

/**
 * @Brief : run sta
 * @param  path path is not a must, if path is empty, using the DB Config output
 * path
 * @return true
 * @return false
 */
bool StaIO::initSTA(std::string path)
{
  /// init
  setStaWorkDirectory();
  std::vector<std::string> paths;
  runLiberty(paths);
  readIdb();
  runSDC();

  return true;
}

/**
 * @brief Judge whether timing_engine is init.
 *
 * @return bool
 */
bool StaIO::isInitSTA()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  bool is_ok = timing_engine->isBuildGraph();
  return is_ok;
}

/**
 * @brief build the graph data.
 *
 * @return unsigned
 */
unsigned StaIO::buildGraph()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  unsigned is_ok = timing_engine->buildGraph();
  return is_ok;
}

/**
 * @Brief : run sta
 * @param  path path is not a must, if path is empty, using the DB Config output
 * path
 * @return true
 * @return false
 */
bool StaIO::runSTA(std::string path)
{
  /// run
  reportTiming();

  return true;
}

/**
 * @brief update the timing data.
 *
 * @return unsigned return 1 if success, else return 0.
 */
unsigned StaIO::updateTiming()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  auto* ista = timing_engine->get_ista();
  unsigned is_ok = ista->updateTiming();
  return is_ok;
}

/**
 * @Brief : read data from idb
 * @param  idb_builder
 * @return true
 * @return false
 */
bool StaIO::readIdb(idb::IdbBuilder* idb_builder)
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  /// idb builder
  idb::IdbBuilder* builder = idb_builder == nullptr ? dmInst->get_idb_builder() : idb_builder;
  auto db_adapter = std::make_unique<ista::TimingIDBAdapter>(timing_engine->get_ista());
  db_adapter->set_idb(builder);
  db_adapter->convertDBToTimingNetlist();
  timing_engine->set_db_adapter(std::move(db_adapter));

  return true;
}
/**
 * @Brief : read SDC
 * @param  path
 * @return true
 * @return false
 */
bool StaIO::runSDC(std::string path)
{
  /// get parameters from db config
  auto db_config = dmInst->get_config();

  /// sdc
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();

  auto sdc_path = db_config.get_sdc_path();
  timing_engine->readSdc(sdc_path.c_str());

  return true;
}

bool StaIO::runSpef(std::string path)
{
  /// get parameters from db config
  auto db_config = dmInst->get_config();

  /// sdc
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();

  auto spef_path = db_config.get_spef_path();
  timing_engine->readSpef(spef_path.c_str());

  return true;
}
/**
 * @Brief : read liberty
 * @param  paths
 * @return true
 * @return false
 */
bool StaIO::runLiberty(std::vector<std::string> paths)
{
  /// get parameters from db config
  auto db_config = dmInst->get_config();

  /// lib
  auto lib_paths = paths.size() > 0 ? paths : db_config.get_lib_paths();

  /// read lib
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  timing_engine->readLiberty(lib_paths);

  return true;
}
/**
 * @Brief : set sta work path
 * @param  path
 * @return true
 * @return false
 */
bool StaIO::setStaWorkDirectory(std::string path)
{  /// get parameters from db config
  auto db_config = dmInst->get_config();

  /// output path
  std::string design_work_space = path.empty() ? db_config.get_output_path() + "/sta" : path;

  /// set directory
  std::filesystem::create_directories(design_work_space);

  /// set output path to timing engine
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  timing_engine->set_design_work_space(design_work_space.c_str());

  return true;
}

/**
 * @Brief : report timing log
 * @return true
 * @return false
 */
bool StaIO::reportTiming()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();

  timing_engine->buildGraph();
  timing_engine->updateTiming();
  timing_engine->reportTiming();

  return true;
}
/**
 * @brief get all the clock net name list for this design
 *
 * @return std::vector<std::string>
 */
std::vector<std::string> StaIO::getClockNetNameList()
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  std::vector<std::string> clock_net_name_list = timing_engine->getClockNetNameList();

  return clock_net_name_list;
}

/**
 * @brief get all the clock name list for this design
 *
 * @return std::vector<std::string>
 */
std::vector<std::string> StaIO::getClockNameList()
{
  std::vector<std::string> clock_name_list;
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  for (auto* sta_clock : timing_engine->getClockList()) {
    clock_name_list.push_back(sta_clock->get_clock_name());
  }
  return clock_name_list;
}

/**
 * @brief get the cell type of the cell.
 *
 * @param cell_name
 * @return std::string
 */
std::string StaIO::getCellType(const char* cell_name)
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  std::string cell_type = timing_engine->getCellType(cell_name);
  return cell_type;
}

/**
 * @brief check if the net is a clock net.
 *
 * @param net_name
 * @return true
 * @return false
 */
bool StaIO::isClockNet(string net_name)
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  bool is_clock_net = timing_engine->isClockNet(net_name.c_str());
  return is_clock_net;
}

/**
 * @brief check if the instance is a sequential cell.
 *
 * @param net_name
 * @return true
 * @return false
 */
bool StaIO::isSequentialCell(std::string instance_name)
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  bool is_sequential_cell = timing_engine->isSequentialCell(instance_name.c_str());
  return is_sequential_cell;
}

/**
 * @brief insert buffer.
 *
 * @param source_sink_net raw_net(source_net),new_net(sink_net).
 * @param sink_pin_list pins to which the new net is connected.
 * @param master_inst_buffer master(buffer type),inst(buffer name).
 * @param buffer_center_loc the center location of the buffer.
 * @param connect_type the connect type of the new net.
 * @return true
 * @return false
 */
bool StaIO::insertBuffer(std::pair<std::string, std::string>& source_sink_net, std::vector<std::string>& sink_pin_list,
                         std::pair<std::string, std::string>& master_inst_buffer, std::pair<int, int> buffer_center_loc,
                         idb::IdbConnectType connect_type)
{
  auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
  auto idb_adapter = std::make_unique<TimingIDBAdapter>(timing_engine->get_ista());
  if (!idb_adapter->get_idb()) {
    idb_adapter->set_idb(dmInst->get_idb_builder());
  }

  // step 1: disconnect
  for (const auto& sink_pin_name : sink_pin_list) {
    auto design_netlist = timing_engine->get_netlist();
    auto [instance_name, instance_pin_name] = Str::splitTwoPart(sink_pin_name.c_str(), "/:");
    if (!instance_pin_name.empty()) {
      // instance pin
      Instance* instance = design_netlist->findInstance(instance_name.c_str());

      LOG_FATAL_IF(!instance) << "instance: " << instance->getFullName() << " not found.";
      std::optional<Pin*> pin = instance->getPin(instance_pin_name.c_str());
      if (pin) {
        idb_adapter->disconnectPin(*pin);
      } else {
        LOG_FATAL << "pin: " << (*pin)->getFullName() << " not found.";
      }
    } else {
      // port
      auto* port = design_netlist->findPort(sink_pin_name.c_str());
      idb_adapter->disconnectPinPort(port);
    }
  }

  // step 2: make instance
  LibertyCell* insert_buf_cell = timing_engine->findLibertyCell(master_inst_buffer.first.c_str());
  Instance* buffer = idb_adapter->makeInstance(insert_buf_cell, master_inst_buffer.second.c_str());
  LibertyPort *input, *output;
  insert_buf_cell->bufferPorts(input, output);

  // step 3: make net
  Net* source_net = timing_engine->findNet(source_sink_net.first.c_str());
  Net* sink_net = idb_adapter->makeNet(source_sink_net.second.c_str(), sink_pin_list, connect_type);

  // step 4: connect
  Pin* insert_buf_in = idb_adapter->connect(buffer, input->get_port_name(), source_net);
  Pin* insert_buf_out = idb_adapter->connect(buffer, output->get_port_name(), sink_net);
  LOG_FATAL_IF(!insert_buf_in);
  LOG_FATAL_IF(!insert_buf_out);

  // step 5: set buffer location
  IdbInstance* idb_buffer = idb_adapter->staToDb(buffer);
  idb_buffer->set_status_placed();
  idb_buffer->set_coodinate(buffer_center_loc.first, buffer_center_loc.second);

  // step 6: update timing engine netlist
  timing_engine->insertBuffer(buffer->get_name());

  idb_buffer->set_orient(IdbOrient::kN_R0);

  return insert_buf_in && insert_buf_out ? true : false;
}

// void StaIO::updateTiming(){
//   auto* timing_engine = ista::TimingEngine::getOrCreateTimingEngine();
//   timing_engine->buildGraph();
//   timing_engine->updateTiming();
// }

}  // namespace iplf
