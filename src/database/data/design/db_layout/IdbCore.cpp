/**
 * @project		iDB
 * @file		IdbCore.h
 * @date		25/05/2021
 * @version		0.1
* @description


        Describe Core information,.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IdbCore.h"

namespace idb {

IdbCoreSide IdbCore::get_side(IdbCoordinate<int32_t>* coordinate)
{
  IdbRect* bounding_box = get_bounding_box();
  if (bounding_box->containPoint(coordinate)) {
    return IdbCoreSide::kNone;
  }

  if (coordinate->get_x() < bounding_box->get_low_x()) {
    return IdbCoreSide::kLeft;
  } else if (coordinate->get_x() > bounding_box->get_high_x()) {
    return IdbCoreSide::kRight;
  } else if (coordinate->get_y() < bounding_box->get_low_y()) {
    return IdbCoreSide::kBottom;
  } else if (coordinate->get_y() > bounding_box->get_high_y()) {
    return IdbCoreSide::kTop;
  } else {
    return IdbCoreSide::kNone;
  }
}

}  // namespace idb
