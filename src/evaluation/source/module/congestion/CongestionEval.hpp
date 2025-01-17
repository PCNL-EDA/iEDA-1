#ifndef SRC_EVALUATOR_SOURCE_CONGESTION_CONGESTIONEVAL_HPP_
#define SRC_EVALUATOR_SOURCE_CONGESTION_CONGESTIONEVAL_HPP_

#include <map>

#include "CongBin.hpp"
#include "CongInst.hpp"
#include "CongNet.hpp"
#include "CongTile.hpp"

namespace eval {

class CongestionEval
{
 public:
  CongestionEval() = default;
  ~CongestionEval() = default;

  void reportCongestion(const std::string& plot_path, const std::string& output_file_name);

  /*----evaluate pin number----*/
  void mapInst2Bin();
  void evalPinNum();
  void reportPinNum();
  void plotPinNum(const std::string& plot_path, const std::string& output_file_name);
  int getBinPinNum(const int& index_x, const int& index_y);
  double getBinPinDens(const int& index_x, const int& index_y);
  std::vector<float> evalPinDens();
  /*----evaluate inst density----*/
  void evalInstDens();
  void reportInstDens();
  void plotInstDens(const std::string& plot_path, const std::string& output_file_name);
  double getBinInstDens(const int& index_x, const int& index_y);
  std::vector<float> getInstDens();

  /*----evaluate net congestion----*/
  void mapNet2Bin();
  void evalNetCong(const std::string& rudy_type);
  void reportNetCong();
  void plotNetCong(const std::string& plot_path, const std::string& output_file_name, const std::string& type);
  double getBinNetCong(const int& index_x, const int& index_y, const std::string& rudy_type);
  std::vector<float> getNetCong(const std::string& rudy_type);
  /*----evaluate post-route congestion----*/
  // void evalViaDens();
  // void reportViaDens();
  // void plotViaDens(const std::string& plot_path, const std::string& output_file_name);
  // double getBinViaDens(const int& index_x, const int& index_y);
  // std::vector<CongNet>& getPostRouteNetlist();

  /*----evaluate routing congestion----*/
  std::vector<float> evalRouteCong();
  float evalACE(const std::vector<float>& hor_edge_cong_list, const std::vector<float>& ver_edge_cong_list);
  std::vector<int> evalOverflow();                 // <TOF,MOF>
  std::vector<int> evalOverflow(int layer_index);  // <TOF,MOF>
  std::vector<float> getUseCapRatioList();
  // produce layer_num maps: raw overflow
  void plotGRCong(const std::string& plot_path, const std::string& output_file_name);
  void plotGRCongOneLayer(const std::string& plot_path, const std::string& output_file_name, int layer_index);
  // produce two maps: TOF,MOF
  void plotOverflow(const std::string& plot_path, const std::string& output_file_name);
  void plotOverflow(const std::string& plot_path, const std::string& output_file_name, const std::vector<int>& plane_grid, const int& x_cnt,
                    const std::string& type);

  void reportCongMap();

  /*----Common used----*/
  void set_tile_grid(TileGrid* tile_grid) { _tile_grid = tile_grid; }
  void set_tile_grid(const int& lx, const int& ly, const int& tileCntX, const int& tileCntY, const int& tileSizeX, const int& tileSizeY,
                     const int& numRoutingLayers);
  void set_cong_grid(CongGrid* cong_grid) { _cong_grid = cong_grid; }
  void set_cong_grid(const int& lx, const int& ly, const int& binCntX, const int& binCntY, const int& binSizeX, const int& binSizeY);
  void set_cong_inst_list(const std::vector<CongInst*>& cong_inst_list) { _cong_inst_list = cong_inst_list; }
  void set_cong_net_list(const std::vector<CongNet*>& cong_net_list) { _cong_net_list = cong_net_list; }

  CongGrid* get_cong_grid() const { return _cong_grid; }
  TileGrid* get_tile_grid() const { return _tile_grid; }
  std::vector<CongInst*>& get_cong_inst_list() { return _cong_inst_list; }
  std::vector<CongNet*>& get_cong_net_list() { return _cong_net_list; }

  void checkRUDYType(const std::string& rudy_type);
  void reportTileGrid();
  void reportCongGrid();

 private:
  TileGrid* _tile_grid = nullptr;
  CongGrid* _cong_grid = nullptr;
  std::vector<CongInst*> _cong_inst_list;
  std::vector<CongNet*> _cong_net_list;

  int32_t getOverlapArea(CongBin* bin, CongInst* inst);
  int32_t getOverlapArea(CongBin* bin, CongNet* net);

  double getRudy(CongBin* bin, CongNet* net);
  double getRudyDev(CongBin* bin, CongNet* net);
  double getPinRudy(CongBin* bin, CongNet* net);
  double getPinSteinerRudy(CongBin* bin, CongNet* net, const std::map<std::string, int64_t>& map);
  double getSteinerRudy(CongBin* bin, CongNet* net, const std::map<std::string, int64_t>& map);
  double getTrueRudy(CongBin* bin, CongNet* net, const std::map<std::string, int64_t>& map);

  float getUsageCapacityRatio(Tile* tile);
};

}  // namespace eval

#endif  // SRC_EVALUATOR_SOURCE_CONGESTION_CONGESTIONEVAL_HPP_
