/*
 * @Author: S.J Chen
 * @Date: 2022-10-24 21:17:18
 * @LastEditors: Shijian Chen  chenshj@pcl.ac.cn
 * @LastEditTime: 2023-03-11 14:20:43
 * @FilePath: /irefactor/src/operation/iPL/api/iPL_API.hh
 * @Description: Interface of iPL.
 */

#ifndef IPL_API_H
#define IPL_API_H

#include "ids.hh"

namespace ipl {

#define iPLAPIInst ipl::iPL_API::getInst()

class iPL_API
{
 public:
  static iPL_API& getInst();
  static void destoryInst();

  void initAPI(std::string pl_json_path, idb::IdbBuilder* idb_builder);
  void runFlow();
  void runIncrementalFlow();
  void insertLayoutFiller();

  void runGP();
  void runMP();
  bool runLG();
  bool runIncrLG();
  bool runIncrLG(std::vector<std::string> inst_name_list);
  void runDP();
  void runBufferInsertion();
  void writeBackSourceDataBase();

  void updatePlacerDB();
  void updatePlacerDB(std::vector<std::string> inst_list);

  std::vector<Rectangle<int32_t>> obtainAvailableWhiteSpaceList(std::pair<int32_t, int32_t> row_range,
                                                                std::pair<int32_t, int32_t> site_range);
  bool checkLegality();

  void reportPLInfo();
  void reportTopoInfo();
  void reportSTWLInfo(std::ofstream& feed);
  void reportHPWLInfo(std::ofstream& feed);
  void reportLongNetInfo(std::ofstream& feed);
  void reportLayoutInfo(std::ofstream& feed);
  void reportPeakBinDensity(std::ofstream& feed);
  void reportOverlapInfo(std::ofstream& feed);
  void reportLayoutWhiteInfo();
  void reportTimingInfo(std::ofstream& feed);
  void reportCongestionInfo();

  bool isSTAStarted();
  bool isPlacerDBStarted();
  bool isAbucasLGStarted();

  // The following interfaces are only for iPL internal calls !
  // The following interfaces are only for iPL internal calls !
  // The following interfaces are only for iPL internal calls !

  void printHPWLInfo();
  void saveNetPinInfoForDebug(std::string path);
  void savePinListInfoForDebug(std::string path);
  void plotConnectionForDebug(std::vector<std::string> net_name_list, std::string path);
  void plotModuleListForDebug(std::vector<std::string> module_prefix_list, std::string path);
  void plotModuleStateForDebug(std::vector<std::string> special_inst_list, std::string path);

  void initSTA();
  void updateSTATiming();
  std::vector<std::string> obtainClockNameList();
  bool isClockNet(std::string net_name);
  bool isSequentialCell(std::string inst_name);
  bool isBufferCell(std::string cell_name);
  void updateSequentialProperty();

  bool insertSignalBuffer(std::pair<std::string, std::string> source_sink_net, std::vector<std::string> sink_pin_list,
                          std::pair<std::string, std::string> master_inst_buffer, std::pair<int, int> buffer_center_loc);

  /*****************************Timing-driven Placement: START*****************************/
  void initTimingEval();
  double obtainPinEarlySlack(std::string pin_name);
  double obtainPinLateSlack(std::string pin_name);
  double obtainPinEarlyArrivalTime(std::string pin_name);
  double obtainPinLateArrivalTime(std::string pin_name);
  double obtainPinEarlyRequiredTime(std::string pin_name);
  double obtainPinLateRequiredTime(std::string pin_name);
  double obtainWNS(const char* clock_name, ista::AnalysisMode mode);
  double obtainTNS(const char* clock_name, ista::AnalysisMode mode);
  void updateTiming();
  void updateTimingInstMovement(std::map<std::string, std::vector<std::pair<Point<int32_t>, Point<int32_t>>>> influenced_net_map,
                                std::vector<std::string> moved_inst_list);
  void destroyTimingEval();
  /*****************************Timing-driven Placement: END*****************************/

  /*****************************Congestion-driven Placement: START*****************************/
  void runRoutabilityGP();
  void initCongestionEval();
  std::vector<float> obtainPinDens();
  std::vector<float> obtainNetCong(std::string rudy_type);
  std::vector<float> evalGRCong();
  std::vector<float> getUseCapRatioList();
  void plotCongMap(const std::string& plot_path, const std::string& output_file_name);
  void destroyCongEval();
  /*****************************Congestion-driven Placement: END*****************************/

 private:
  static iPL_API* _ipl_api_instance;

  iPL_API() = default;
  iPL_API(const iPL_API&) = delete;
  iPL_API(iPL_API&&) = delete;
  ~iPL_API();
  iPL_API& operator=(const iPL_API&) = delete;
  iPL_API& operator=(iPL_API&&) = delete;
};

}  // namespace ipl

#endif  // IPL_API_H
