#pragma once
/**
 * @File Name: irt_io.h
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-17
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <string>
#include <vector>

#include "file_placement.h"

namespace iplf {
#define plInst PlacerIO::getInstance()

class PlacerIO
{
 public:
  static PlacerIO* getInstance()
  {
    if (!_instance) {
      _instance = new PlacerIO;
    }
    return _instance;
  }

  ////
  int32_t get_dp_index() { return _dp_index; }
  void set_dp_index(int32_t index) { _dp_index = index; }
  void resetDpIndex() { _dp_index = 0; }
  void initFileInstanceSize(int64_t size) { _file_inst_list.reserve(size); }
  std::vector<FileInstance>& get_file_inst_list() { return _file_inst_list; }
  void set_file_inst_list(std::vector<FileInstance>& inst_list) { _file_inst_list = inst_list; }
  void addFileInstance(FileInstance file_inst) { _file_inst_list.emplace_back(file_inst); }
  void addFileInstance(std::string name, int32_t x, int32_t y, int8_t orient)
  {
    FileInstance file_inst;
    memset(&file_inst, 0, sizeof(FileInstance));
    // memcpy(file_inst.instance_name, name.c_str(), name.length());
    file_inst.x = x;
    file_inst.y = y;
    file_inst.orient = orient;
    _file_inst_list.emplace_back(file_inst);
  }
  void clearFileInstanceList() { _file_inst_list.clear(); }

  /// io
  void initPlacer(std::string config);
  void destroyPlacer();
  bool runPlacement();
  bool runIncrementalLegalization();
  bool runIncrementalLegalization(std::vector<std::string>& changed_inst_list);
  bool runFillerInsertion(std::string config);
  bool runMacroPlacement();
  bool runGlobalPlacement();
  bool runLegalization();
  void runDetailPlacement();

  bool checkLegality();
  bool reportPlacement();

  bool runIncrementalFlow(std::string config);

  bool readInstanceDataFromDirectory(std::string directory = "");
  bool saveInstanceDataToDirectory(std::string directory);

  int64_t getFileInstanceBufferSize();
  bool readInstanceDataFromFile(std::string path = "");
  bool saveInstanceDataToFile(std::string path);

 private:
  static PlacerIO* _instance;

  int32_t _dp_index = 0;
  std::string _directory = "";
  std::vector<FileInstance> _file_inst_list;

  PlacerIO() {}
  ~PlacerIO() = default;
};

}  // namespace iplf
