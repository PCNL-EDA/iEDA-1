#include "RT.hpp"

#include "Logger.hpp"

namespace irt {

// public

void RT::initInst(std::map<std::string, std::any>& config_map, idb::IdbBuilder* idb_builder)
{
  if (_rt_instance == nullptr) {
    _rt_instance = new RT(config_map, idb_builder);
  }
}

RT& RT::getInst()
{
  if (_rt_instance == nullptr) {
    LOG_INST.error(Loc::current(), "The instance not initialized!");
  }
  return *_rt_instance;
}

void RT::destroyInst()
{
  if (_rt_instance != nullptr) {
    delete _rt_instance;
    _rt_instance = nullptr;
  }
}

DataManager& RT::getDataManager()
{
  return _data_manager;
}

// private

RT* RT::_rt_instance = nullptr;

void RT::init()
{
  Logger::initInst();
  printHeader();
  LOG_INST.printLogFilePath();
  _data_manager.input(_config_map, _idb_builder);
}

void RT::printHeader()
{
  // clang-format off
  LOG_INST.info(Loc::current(), ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  LOG_INST.info(Loc::current(), "_____ ________ ________     _______________________ ________ ________  ");
  LOG_INST.info(Loc::current(), "___(_)___  __ \\___  __/     __  ___/___  __/___    |___  __ \\___  __/");
  LOG_INST.info(Loc::current(), "__  / __  /_/ /__  /        _____ \\ __  /   __  /| |__  /_/ /__  /    ");
  LOG_INST.info(Loc::current(), "_  /  _  _, _/ _  /         ____/ / _  /    _  ___ |_  _, _/ _  /      ");
  LOG_INST.info(Loc::current(), "/_/   /_/ |_|  /_/          /____/  /_/     /_/  |_|/_/ |_|  /_/       ");
  LOG_INST.info(Loc::current(), ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  // clang-format on  
  sleep(2);   
}

void RT::destroy()
{
  _data_manager.output(_idb_builder);
  LOG_INST.printLogFilePath();
  printFooter();
  Logger::destroyInst();
}

void RT::printFooter()
{
  // clang-format off
  LOG_INST.info(Loc::current(), ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  LOG_INST.info(Loc::current(), "_____ ________ ________     _______________________   ________________________  __  ");
  LOG_INST.info(Loc::current(), "___(_)___  __ \\___  __/     ___  ____/____  _/___  | / /____  _/__  ___/___  / / / ");
  LOG_INST.info(Loc::current(), "__  / __  /_/ /__  /        __  /_     __  /  __   |/ /  __  /  _____ \\ __  /_/ /  ");
  LOG_INST.info(Loc::current(), "_  /  _  _, _/ _  /         _  __/    __/ /   _  /|  /  __/ /   ____/ / _  __  /    ");
  LOG_INST.info(Loc::current(), "/_/   /_/ |_|  /_/          /_/       /___/   /_/ |_/   /___/   /____/  /_/ /_/     ");
  LOG_INST.info(Loc::current(), ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  // clang-format on  
  sleep(2);
}

}  // namespace irt
