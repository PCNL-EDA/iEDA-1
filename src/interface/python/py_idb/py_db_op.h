#pragma once

#include <idm.h>
#include <report_manager.h>

#include "IdbEnum.h"
#include "IdbInstance.h"

namespace python_interface {
bool setNet(const std::string& net_name, const std::string& type)
{
  return dmInst->setNetType(net_name, type);
}

bool removeExceptPgNet()
{
  dmInst->removeBlockageExceptPGNet();
}

bool clearBlockage(const std::string& type)
{
  dmInst->clearBlockage(type);
}

bool idbGet(const std::string& inst_name, const std::string& net_name, const std::string& file_name)
{
  bool ok = false;
  if (not inst_name.empty()) {
    ok |= rptInst->reportInstance(file_name, inst_name);
  }
  if (not net_name.empty()) {
    ok |= rptInst->reportNet(file_name, net_name);
  }
  return ok;
}

bool idbDeleteInstance(const std::string& inst_name)
{
  bool deleted = dmInst->get_idb_design()->get_instance_list()->remove_instance(inst_name);
  return deleted;
}

bool idbDeleteNet(const std::string& net_name)
{
  bool deleted = dmInst->get_idb_design()->get_net_list()->remove_net(net_name);
  return deleted;
}

bool idbCreateInstance(const std::string& inst_name, const std::string& cell_master, int coord_x, int coord_y, const std::string& orient,
                       const std::string& type, const std::string& status)
{
  auto* enumInst = IdbEnum::GetInstance();
  IdbOrient orient_enum = enumInst->get_site_property()->get_orient_value(orient);
  IdbInstanceType type_enum = enumInst->get_instance_property()->get_type(type);
  IdbPlacementStatus status_enum = status.empty() ? IdbPlacementStatus::kUnplaced : enumInst->get_instance_property()->get_status(status);

  IdbInstance* inst = dmInst->createInstance(inst_name, cell_master, coord_x, coord_y, orient_enum, type_enum, status_enum);
  return inst;
}

bool idbCreateNet(const std::string& net_name, const std::string& conn_type){
  IdbConnectType type = IdbEnum::GetInstance()->get_connect_property()->get_type(conn_type);
  IdbNet* net = dmInst->createNet(net_name, type);
  return net;
}


}  // namespace python_interface