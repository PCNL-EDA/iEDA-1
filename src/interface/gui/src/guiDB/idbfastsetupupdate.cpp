#include "guiConfig.h"
#include "idbfastsetup.h"

bool IdbSpeedUpSetup::updateInstance() {
  _gui_design->clearUpdateItemList();

  auto& inst_list = _design->get_instance_list()->get_instance_list();
  for (int i = _item_index; i < inst_list.size(); i++) {
    if (i >= _item_index + 1000 || i >= inst_list.size()) {
      break;
    }

    auto instance = inst_list[i];
    if (instance == nullptr || instance->get_cell_master() == nullptr) {
      continue;
    }

    createInstanceCore(instance);
    createInstancePad(instance);
    createInstanceBlock(instance);
  }
  _item_index += 1000;

  _gui_design->update();

  if (_item_index >= inst_list.size()) {
    return true;
  } else {
    return false;
  }
}

bool IdbSpeedUpSetup::updateNet() {
  _gui_design->clearUpdateItemList();

  auto& net_list = _design->get_net_list()->get_net_list();
  for (int i = _item_index; i < net_list.size(); i++) {
    if (i >= _item_index + 100 || i >= net_list.size()) {
      break;
    }

    auto net                    = net_list[i];
    GuiSpeedupItemType gui_type = getNetGuiType(net);
    for (IdbRegularWire* wire : net->get_wire_list()->get_wire_list()) {
      for (IdbRegularWireSegment* segment : wire->get_segment_list()) {
        if (segment->is_via()) {
          /// find gui via list ptr
          createNetVia(segment, gui_type);

          /// if point >=2 means wire + via
          if (segment->get_point_number() >= 2) {
            GuiSpeedupItem* this_gui_wire = findNetItem(segment, gui_type);
            if (this_gui_wire == nullptr) {
              continue;
            }
            this_gui_wire->set_type(gui_type);

            createNetPoints(segment, this_gui_wire);

            _gui_design->addUpdateItem(this_gui_wire);
          }
        } else if (segment->is_rect()) {
          /// find gui wire list ptr
          GuiSpeedupItem* this_gui_wire = findNetItem(segment, gui_type);
          if (this_gui_wire == nullptr) {
            continue;
          }
          this_gui_wire->set_type(gui_type);

          createNetRect(segment, this_gui_wire);

          _gui_design->addUpdateItem(this_gui_wire);
        } else {
          /// find gui wire list ptr
          GuiSpeedupItem* this_gui_wire = findNetItem(segment, gui_type);
          if (this_gui_wire == nullptr) {
            continue;
          }
          this_gui_wire->set_type(gui_type);

          createNetPoints(segment, this_gui_wire);

          _gui_design->addUpdateItem(this_gui_wire);
        }
      }
    }
  }

  _item_index += 100;

  _gui_design->update();

  if (_item_index >= net_list.size()) {
    return true;
  } else {
    return false;
  }
}
