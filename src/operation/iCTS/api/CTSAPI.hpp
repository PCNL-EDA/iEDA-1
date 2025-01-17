#pragma once

#include <any>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "../../../database/interaction/ids.hpp"

namespace icts {

#define CTSAPIInst (icts::CTSAPI::getInst())

using ieda::Log;
using ieda::Str;
using ieda::Time;
using SkewConstraintsMap = std::map<std::pair<std::string, std::string>, std::pair<double, double>>;

template <typename T>
concept Stringable = requires(const T& t) {
                       {
                         std::to_string(t)
                         } -> std::convertible_to<std::string>;
                     };

class CTSAPI
{
 public:
  static CTSAPI& getInst();
  static void destroyInst();
  // open API
  void runCTS();
  void writeDB();
  void writeGDS();
  void report(const std::string& save_dir);
  // flow API
  void resetAPI();
  void init(const std::string& config_file);
  void readData();
  void routing();
  void synthesis();
  void evaluate();
  void balance();
  void optimize();
  void getClockNets(std::map<std::string, std::vector<CtsSignalWire>>& net_topo_map) const;
  icts::CtsConfig* get_config() { return _config; }
  icts::CtsDesign* get_design() { return _design; }
  icts::CtsDBWrapper* get_db_wrapper() { return _db_wrapper; }
  // Timing
  void addTimingNode(TimingNode* node);
  TimingNode* findTimingNode(const std::string& name);
  // iSTA
  void dumpVertexData(const std::vector<std::string>& vertex_names) const;
  double getClockUnitCap() const;
  double getClockUnitRes() const;
  double getSinkCap(icts::CtsInstance* sink) const;
  bool isFlipFlop(const std::string& inst_name) const;
  bool isClockNet(const std::string& net_name) const;
  void startDbSta();
  void readClockNetNames() const;
  void setPropagateClock();
  void convertDBToTimingEngine();
  void reportTiming() const;
  void refresh();
  icts::CtsPin* findDiverPin(icts::CtsNet* net);
  std::map<std::string, double> elmoreDelay(const icts::EvalNet& eval_net);
  std::vector<std::vector<double>> queryCellLibIndex(const std::string& cell_master, const std::string& query_field,
                                                     const std::string& from_port = "", const std::string& to_port = "");
  std::vector<double> queryCellLibValue(const std::string& cell_master, const std::string& query_field, const std::string& from_port = "",
                                        const std::string& to_port = "");
  icts::CtsCellLib* getCellLib(const std::string& cell_masterconst, const std::string& from_port = "", const std::string& to_port = "",
                               const bool& use_work_value = true);
  std::vector<icts::CtsCellLib*> getAllBufferLibs();
  std::vector<std::string> getMasterClocks(icts::CtsNet* net) const;
  double getClockAT(const std::string& pin_name, const std::string& belong_clock_name) const;
  std::string getCellType(const std::string& cell_master) const;
  double getCellArea(const std::string& cell_master) const;
  double getCellCap(const std::string& cell_master) const;
  double getSlewIn(const std::string& pin_name) const;
  double getCapOut(const std::string& pin_name) const;
  std::vector<double> solvePolynomialRealRoots(const std::vector<double>& coeffs);

  // iRT
  void iRTinit();
  void routingWire(icts::CtsNet* net);
  void iRTdestroy();

  // iTO
  std::vector<idb::IdbNet*> fix(const icts::OptiNet& opti_net);
  void makeTopo(ito::Tree* topo, const icts::OptiNet& opti_net) const;

  // synthesis
  bool isInDie(const icts::Point& point) const;
  void placeInstance(icts::CtsInstance* inst);
  idb::IdbInstance* makeIdbInstance(const std::string& inst_name, const std::string& cell_master);
  idb::IdbNet* makeIdbNet(const std::string& net_name);
  void linkIdbNetToSta(idb::IdbNet* idb_net);
  void disconnect(idb::IdbPin* pin);
  void connect(idb::IdbInstance* idb_inst, const std::string& pin_name, idb::IdbNet* net);
  void insertBuffer(const std::string& name);
  int genId();

  // evaluate
  bool isTop(const std::string& net_name) const;
  void buildRCTree(const std::vector<icts::EvalNet>& eval_nets);
  void buildRCTree(const icts::EvalNet& eval_net);
  void resetRCTree(const std::string& net_name);

  // useful skew
  void buildLogicRCTree(const std::vector<icts::EvalNet>& eval_nets);
  void buildLogicRCTree(const icts::EvalNet& eval_net);
  SkewConstraintsMap skewConstraints() const;
  SkewConstraintsMap fixSkewConstraints() const;

  // log
  void checkFile(const std::string& dir, const std::string& file_name, const std::string& suffix = ".rpt") const;

  template <Stringable T>
  std::string stringify(const T& t)
  {
    return std::to_string(t);
  }

  std::string stringify(const std::string_view sv) { return std::string(sv); }

  template <typename... Args>
  std::string toString(const Args&... args)
  {
    return (stringify(args) + ...);
  }

  template <typename... Args>
  void saveToLog(const Args&... args)
  {
    (*_log_ofs) << toString(args...) << std::endl;
  }

  // debug
  void writeVerilog() const;
  void toPyArray(const icts::Point& point, const std::string& label);
  void toPyArray(const icts::Segment& segment, const std::string& label);
  void toPyArray(const icts::Polygon& polygon, const std::string& label);
  void toPyArray(const icts::CtsPolygon<int64_t>& polygon, const std::string& label);

// python API
#ifdef PY_MODEL

#ifdef USE_EXTERNAL_MODEL
  icts::ModelBase* findExternalModel(const std::string& net_name);
#endif

  icts::ModelBase* fitPyModel(const std::vector<std::vector<double>>& X, const std::vector<double>& y, const icts::FitType& fit_type);
  void saveFig(const std::string& file_name);
  void plot(const icts::Point& point, const std::string& label);
  void plot(const icts::Segment& segment, const std::string& label);
  void plot(const icts::Polygon& polygon, const std::string& label);
  void plot(const icts::CtsPolygon<int64_t>& polygon, const std::string& label);
#endif
 private:
  static CTSAPI* _cts_api_instance;
  CTSAPI() = default;
  CTSAPI(const CTSAPI& other) = delete;
  CTSAPI(CTSAPI&& other) = delete;
  ~CTSAPI() = default;
  CTSAPI& operator=(const CTSAPI& other) = delete;
  CTSAPI& operator=(CTSAPI&& other) = delete;
  // function
  std::vector<std::string> splitString(std::string str, const char split);
  // private STA
  void readSTAFile();
  ista::RctNode* makeRCTreeNode(const icts::EvalNet& eval_net, const std::string& name);
  ista::RctNode* makeLogicRCTreeNode(icts::CtsPin* pin);
  ista::DesignObject* findStaPin(icts::CtsPin* pin) const;
  ista::DesignObject* findStaPin(const std::string& pin_full_name) const;
  ista::Net* findStaNet(const icts::EvalNet& eval_net) const;
  ista::Net* findStaNet(const std::string& name) const;
  double getUnitCap() const;
  double getUnitRes() const;
  double getCapacitance(const double& wire_length, const int& level) const;
  double getResistance(const double& wire_length, const int& level) const;
  double getCapacitance(const CtsSignalWire& signal_wire, const int& level) const;
  double getResistance(const CtsSignalWire& signal_wire, const int& level) const;
  double getWireLength(const icts::CtsSignalWire& signal_wire) const;
  ista::TimingIDBAdapter* getStaDbAdapter() const;

  // variable
  icts::CtsConfig* _config = nullptr;
  icts::CtsDesign* _design = nullptr;
  icts::CtsDBWrapper* _db_wrapper = nullptr;
  icts::CtsReportTable* _report = nullptr;
  std::ofstream* _log_ofs = nullptr;
  icts::CtsLibs* _libs = nullptr;
  icts::Synthesis* _synth = nullptr;
  icts::Evaluator* _evaluator = nullptr;
  icts::Balancer* _balancer = nullptr;
  icts::ModelFactory* _model_factory = nullptr;
  icts::MplHelper* _mpl_helper = nullptr;
  ista::TimingEngine* _timing_engine = nullptr;
};

}  // namespace icts
