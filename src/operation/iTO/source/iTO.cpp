#include "iTO.h"
#include "DbInterface.h"
#include "CTSViolationFixer.h"
#include "HoldOptimizer.h"
#include "JsonParser.h"
#include "SetupOptimizer.h"
#include "ViolationOptimizer.h"
#include "io/DbInterface.h"

namespace ito {

iTO::iTO(const std::string &config_file) {
  _to_config = new ToConfig;
  JsonParser *json = JsonParser::get_json_parser();
  json->parse(config_file, _to_config);
}

iTO::~iTO() {
  CTSViolationFixer::get_cts_violation_fixer(_db_interface)->destroyCTSViolationFixer();
  _db_interface->destroyDbInterface();
}

void iTO::runTO() {
  if (_to_config->get_optimize_drv()) {
    optimizeDesignViolation();
  }
  if (_to_config->get_optimize_hold()) {
    optimizeHold();
  }
  if (_to_config->get_optimize_setup()) {
    optimizeSetup();
  }
}

void iTO::initialization(idb::IdbBuilder *idb_build, ista::TimingEngine *timing) {
  cout << "\033[1;31m" << endl;
  cout << R"(  _____       _ _     _____        _         )" << endl;
  cout << R"( |_   _|     (_) |   |  __ \      | |        )" << endl;
  cout << R"(   | |  _ __  _| |_  | |  | | __ _| |_ __ _  )" << endl;
  cout << R"(   | | | '_ \| | __| | |  | |/ _` | __/ _` | )" << endl;
  cout << R"(  _| |_| | | | | |_  | |__| | (_| | || (_| | )" << endl;
  cout << R"( |_____|_| |_|_|\__| |_____/ \__,_|\__\__,_| )" << endl;
  cout << R"(                                             )" << endl;
  cout << "\033[0m" << endl;
  _db_interface = ito::DbInterface::get_db_interface(_to_config, idb_build, timing);
};

void iTO::resetInitialization(idb::IdbBuilder    *idb_build,
                              ista::TimingEngine *timing_engine) {
  CTSViolationFixer::get_cts_violation_fixer(_db_interface)->destroyCTSViolationFixer();
  DbInterface::destroyDbInterface();
  _db_interface =
      ito::DbInterface::get_db_interface(_to_config, idb_build, timing_engine);
  CTSViolationFixer::get_cts_violation_fixer(_db_interface);
}

void iTO::optimizeDesignViolation() {
  cout << "\033[1;32m" << endl;
  cout << R"(    ____        _   _           _           _____  _______      __ )"
       << endl;
  cout << R"(   / __ \      | | (_)         (_)         |  __ \|  __ \ \    / / )"
       << endl;
  cout << R"(  | |  | |_ __ | |_ _ _ __ ___  _ _______  | |  | | |__) \ \  / /  )"
       << endl;
  cout << R"(  | |  | | '_ \| __| | '_ ` _ \| |_  / _ \ | |  | |  _  / \ \/ /   )"
       << endl;
  cout << R"(  | |__| | |_) | |_| | | | | | | |/ /  __/ | |__| | | \ \  \  /    )"
       << endl;
  cout << R"(   \____/| .__/ \__|_|_| |_| |_|_/___\___| |_____/|_|  \_\  \/     )"
       << endl;
  cout << R"(         | |                                                       )"
       << endl;
  cout << R"(         |_|                                                       )"
       << endl;
  cout << R"(                                                                   )"
       << endl;
  cout << "\033[0m" << endl;
  ViolationOptimizer *drv_optimizer = new ViolationOptimizer(_db_interface);
  drv_optimizer->fixViolations();
}

void iTO::optimizeHold() {
  cout << "\033[1;33m" << endl;
  cout << R"(   ____        _   _           _           _    _       _     _  )" << endl;
  cout << R"(  / __ \      | | (_)         (_)         | |  | |     | |   | | )" << endl;
  cout << R"( | |  | |_ __ | |_ _ _ __ ___  _ _______  | |__| | ___ | | __| | )" << endl;
  cout << R"( | |  | | '_ \| __| | '_ ` _ \| |_  / _ \ |  __  |/ _ \| |/ _` | )" << endl;
  cout << R"( | |__| | |_) | |_| | | | | | | |/ /  __/ | |  | | (_) | | (_| | )" << endl;
  cout << R"(  \____/| .__/ \__|_|_| |_| |_|_/___\___| |_|  |_|\___/|_|\__,_| )" << endl;
  cout << R"(        | |                                                      )" << endl;
  cout << R"(        |_|                                                      )" << endl;
  cout << R"(                                                                 )" << endl;
  cout << "\033[0m" << endl;
  HoldOptimizer *hold_optimizer = new HoldOptimizer(_db_interface);
  hold_optimizer->optimizeHold();
  // _hold_optimizer->insertHoldDelay("BUF_X1", "_862_:D");
}

void iTO::optimizeSetup() {
  cout << "\033[1;34m" << endl;
  cout << R"(    ____        _   _           _            _____      _                )"
       << endl;
  cout << R"(   / __ \      | | (_)         (_)          / ____|    | |               )"
       << endl;
  cout << R"(  | |  | |_ __ | |_ _ _ __ ___  _ _______  | (___   ___| |_ _   _ _ __   )"
       << endl;
  cout << R"(  | |  | | '_ \| __| | '_ ` _ \| |_  / _ \  \___ \ / _ \ __| | | | '_ \  )"
       << endl;
  cout << R"(  | |__| | |_) | |_| | | | | | | |/ /  __/  ____) |  __/ |_| |_| | |_) | )"
       << endl;
  cout << R"(   \____/| .__/ \__|_|_| |_| |_|_/___\___| |_____/ \___|\__|\__,_| .__/  )"
       << endl;
  cout << R"(         | |                                                     | |     )"
       << endl;
  cout << R"(         |_|                                                     |_|     )"
       << endl;
  cout << R"(                                                                         )"
       << endl;
  cout << "\033[0m" << endl;
  SetupOptimizer *setup_optimizer = new SetupOptimizer(_db_interface);
  setup_optimizer->optimizeSetup();
}

std::vector<idb::IdbNet *> iTO::optimizeCTSDesignViolation(IdbNet *idb_net, Tree *topo) {
  CTSViolationFixer *cts_drv_opt =
      CTSViolationFixer::get_cts_violation_fixer(_db_interface);
  return cts_drv_opt->fixTiming(idb_net, topo);
}

} // namespace ito
