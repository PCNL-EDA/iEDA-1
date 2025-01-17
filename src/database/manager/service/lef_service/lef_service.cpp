/**
 * @project		iDB
 * @file		lef_service.cpp
 * @author		Yell
 * @date		25/05/2021
 * @version		0.1
* @description


        This is a lef db management class to provide db interface, including
read and write operation.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "lef_service.h"

#include <mutex>

namespace idb {

IdbLefService::IdbLefService()
{
  _layout = std::make_shared<IdbLayout>();
}

IdbLefService::~IdbLefService()
{
}

IdbLefServiceResult IdbLefService::LefFileInit(vector<string> lef_files)
{
  vector<string>::iterator it = lef_files.begin();
  for (; it != lef_files.end(); ++it) {
    string filename = *it;
    FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
      std::cout << "Can not open LEF file ( " << filename.c_str() << " )" << std::endl;

      return IdbLefServiceResult::kServiceFailed;
    } else {
      //   std::cout << "Open LEF file success ( " << filename.c_str() << " )"
      //             << std::endl;
    }
  }
  // set lef files
  _lef_files.insert(_lef_files.end(), lef_files.begin(), lef_files.end());

  return IdbLefServiceResult::kServiceSuccess;
}

IdbLayout* IdbLefService::get_layout()
{
  if (!_layout) {
    _layout = std::make_shared<IdbLayout>();
  }

  return _layout.get();
}

IdbCheck* IdbLefService::get_check()
{
  if (!_check) {
    _check = std::make_shared<IdbCheck>();
  }
  return _check.get();
}

}  // namespace idb
