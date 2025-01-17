#include "guiConfig.h"
#include "idbfastsetup.h"
#include "omp.h"

void IdbSpeedUpSetup::showDrc(std::map<std::string, std::vector<idrc::DrcViolationSpot*>>& drc_db, int max_num) {
  for (auto drc : drc_db) {
    auto container = _gui_design->get_drc_container(drc.first);
    if (container != nullptr) {
      int index = 0;

      auto drc_spot_list = drc.second;
      if (max_num > 0) {
        int size = max_num > drc_spot_list.size() ? drc_spot_list.size() : max_num;
        // #pragma omp parallel for
        for (int i = 0; i < size; i++) {
          /// create drc view
          std::string layer = drc_spot_list[i]->get_layer();
          auto drc_list     = container->findDrcList(layer);
          createDrc(drc_list, drc_spot_list[i]);
        }
      } else {
        // #pragma omp parallel for
        for (auto drc_spot : drc_spot_list) {
          /// create drc view
          std::string layer = drc_spot->get_layer();
          auto drc_list     = container->findDrcList(layer);
          createDrc(drc_list, drc_spot);
        }
      }
    }
  }
}

void IdbSpeedUpSetup::createDrc(GuiSpeedupDrcList* drc_list, idrc::DrcViolationSpot* drc_db) {
  if (drc_list == nullptr || drc_db == nullptr) {
    return;
  }

  int min_x = drc_db->get_min_x();
  int min_y = drc_db->get_min_y();
  int max_x = drc_db->get_max_x();
  int max_y = drc_db->get_max_y();

  /// if line
  if (min_x == max_x || min_y == max_y) {
    qreal q_min_x       = _transform.db_to_guidb(min_x);
    qreal q_min_y       = _transform.db_to_guidb_rotate(min_y);
    qreal q_max_x       = _transform.db_to_guidb(max_x);
    qreal q_max_y       = _transform.db_to_guidb_rotate(max_y);
    GuiSpeedupDrc* item = drc_list->findItem(QPointF((q_min_x + q_max_x) / 2, (q_min_y + q_max_y) / 2));
    if (item == nullptr) {
      std::cout << "Error : can not find Drc item in die" << std::endl;
      return;
    }

    item->add_point(QPointF(q_min_x, q_min_y), QPointF(q_max_x, q_max_y));
  } else {
    /// rect
    QRectF rect         = _transform.db_to_guidb_rect(min_x, min_y, max_x, max_y);
    GuiSpeedupDrc* item = drc_list->findItem(rect.center());
    if (item == nullptr) {
      std::cout << "Error : can not find Drc item in die" << std::endl;
      return;
    }
    item->add_rect(rect);
  }
}