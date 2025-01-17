#include "ipl_io.h"

#include "gui_io.h"

namespace igui {

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  bool GuiIO::autoRunPlacer() {
    return true;
    //    return iplf::tmInst->autoRunPlacer();
  }

  void GuiIO::updateInstanceInFastMode(std::vector<iplf::FileInstance>& file_inst_list) {
    return _gui_win->get_scene()->updateInstanceInFastMode(file_inst_list);
  }

  bool GuiIO::guiUpdateInstanceInFastMode(std::string directory, bool b_reset) {
    if (b_reset) {
      iplf::plInst->resetDpIndex();
    }
    if (iplf::plInst->readInstanceDataFromDirectory(directory)) {
      updateInstanceInFastMode(iplf::plInst->get_file_inst_list());
      return true;
    } else {
      return false;
    }

    return false;
  }

}  // namespace igui