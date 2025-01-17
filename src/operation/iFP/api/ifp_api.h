#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>

#define fpApiInst ifp::FpApi::getInstance()
namespace ifp {

class FpApi
{
 public:
  static FpApi* getInstance()
  {
    if (!_instance) {
      _instance = new FpApi;
    }
    return _instance;
  }

  static void destroyInst()
  {
    if (_instance != nullptr) {
      delete _instance;
      _instance = nullptr;
    }
  }

  // function
  bool initDie(double die_lx, double die_ly, double die_ux, double die_uy);
  bool initCore(double core_lx, double core_ly, double core_ux, double core_uy, std::string core_site_name, std::string iocell_site_name,
                std::string corner_site_name);
  bool makeTracks(std::string layer_name, int x_offset, int x_pitch, int y_offset, int y_pitch);
  bool autoPlacePins(std::string layer_name, int width, int height);
  bool placePort(std::string pin_name, int32_t x_offset, int32_t y_offset, int32_t rect_width, int32_t rect_height, std::string layer_name);
  bool placeIOFiller(std::vector<std::string> filler_name_list, std::string prefix, std::string orient, double begin, double end,
                     std::string source);

  void insertTapCells(double distance, std::string tapcell_master_name);
  void insertEndCaps(std::string endcap_master);

 private:
  static FpApi* _instance;

  FpApi() {}
  FpApi(const FpApi& other) = delete;
  FpApi(FpApi&& other) = delete;
  ~FpApi() = default;
  FpApi& operator=(const FpApi& other) = delete;
  FpApi& operator=(FpApi&& other) = delete;
  // function
};

}  // namespace ifp
