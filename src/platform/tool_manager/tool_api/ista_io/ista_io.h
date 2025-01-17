#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

#define staInst iplf::StaIO::getInstance()

namespace ista {
class StaClockTree;
}

namespace idb {
class IdbBuilder;
enum class IdbConnectType : uint8_t;
}  // namespace idb

namespace iplf {

class StaIO
{
 public:
  static StaIO* getInstance()
  {
    if (!_instance) {
      _instance = new StaIO;
    }
    return _instance;
  }

  /// getter

  /// io
  bool autoRunSTA(std::string path = "");
  bool initSTA(std::string path = "");
  bool isInitSTA();
  unsigned buildGraph();
  bool runSTA(std::string path = "");
  unsigned updateTiming();
  bool buildClockTree(std::string sta_path = "");

  std::vector<std::unique_ptr<ista::StaClockTree>>& getClockTree();

  /// operator
  std::vector<std::string> getClockNetNameList();
  std::vector<std::string> getClockNameList();
  std::string getCellType(const char* cell_name);
  bool isClockNet(std::string net_name);
  bool isSequentialCell(std::string instance_name);
  bool insertBuffer(std::pair<std::string, std::string>& source_sink_net, std::vector<std::string>& sink_pin_list,
                    std::pair<std::string, std::string>& master_inst_buffer, std::pair<int, int> buffer_center_loc,
                    idb::IdbConnectType connect_type);

 private:
  static StaIO* _instance;

  StaIO() {}
  ~StaIO() = default;

  bool setStaWorkDirectory(std::string path = "");
  bool readIdb(idb::IdbBuilder* idb_builder = nullptr);
  bool runSDC(std::string path = "");
  bool runLiberty(std::vector<std::string> paths);
  bool runSpef(std::string path = "");
  bool reportTiming();
};

}  // namespace iplf
