#include "VRDataManager.hpp"

#include "RTUtil.hpp"
#include "VRNet.hpp"

namespace irt {

// public

void VRDataManager::input(Config& config, Database& database)
{
  wrapConfig(config);
  wrapDatabase(database);
  buildConfig();
  buildDatabase();
}

std::vector<VRNet> VRDataManager::convertToVRNetList(std::vector<Net>& net_list)
{
  std::vector<VRNet> vr_net_list;
  vr_net_list.reserve(net_list.size());
  for (Net& net : net_list) {
    vr_net_list.emplace_back(convertToVRNet(net));
  }
  return vr_net_list;
}

VRNet VRDataManager::convertToVRNet(Net& net)
{
  VRNet vr_net;
  vr_net.set_origin_net(&net);
  vr_net.set_net_idx(net.get_net_idx());
  for (Pin& pin : net.get_pin_list()) {
    vr_net.get_vr_pin_list().push_back(VRPin(pin));
  }
  vr_net.set_vr_driving_pin(VRPin(net.get_driving_pin()));
  vr_net.set_bounding_box(net.get_bounding_box());
  vr_net.set_dr_result_tree(net.get_dr_result_tree());
  return vr_net;
}

// private

void VRDataManager::wrapConfig(Config& config)
{
  _vr_config.temp_directory_path = config.vr_temp_directory_path;
}

void VRDataManager::wrapDatabase(Database& database)
{
  wrapGCellAxis(database);
  wrapViaMasterList(database);
  wrapRoutingLayerList(database);
}

void VRDataManager::wrapMicronDBU(Database& database)
{
  _vr_database.set_micron_dbu(database.get_micron_dbu());
}

void VRDataManager::wrapGCellAxis(Database& database)
{
  GCellAxis& gcell_axis = _vr_database.get_gcell_axis();
  gcell_axis = database.get_gcell_axis();
}

void VRDataManager::wrapViaMasterList(Database& database)
{
  std::vector<std::vector<ViaMaster>>& layer_via_master_list = _vr_database.get_layer_via_master_list();
  layer_via_master_list = database.get_layer_via_master_list();
}

void VRDataManager::wrapRoutingLayerList(Database& database)
{
  std::vector<RoutingLayer>& routing_layer_list = _vr_database.get_routing_layer_list();
  routing_layer_list = database.get_routing_layer_list();
}

void VRDataManager::buildConfig()
{
}

void VRDataManager::buildDatabase()
{
}

}  // namespace irt
