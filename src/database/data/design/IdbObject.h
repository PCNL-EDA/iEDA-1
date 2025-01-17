#pragma once
/**
 * @project		iDB
 * @file		IdbObject.h
 * @date		25/05/2021
 * @version		0.1
 * @description


        Basic class for all Idb structure.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "../../../basic/geometry/IdbGeometry.h"

namespace idb {

// static uint64_t GLOBAL_ID = 0;
class IdbObject
{
 public:
  IdbObject();
  virtual ~IdbObject();

  // getter
  IdbRect* get_bounding_box() { return _bounding_box; }
  uint64_t& get_id() { return _id; }

  // setter
  bool set_bounding_box(int32_t ll_x, int32_t ll_y, int32_t ur_x, int32_t ur_y);
  void set_bounding_box(IdbRect* bounding_box) { _bounding_box = bounding_box; }
  void set_bounding_box(IdbRect bounding_box);
  void set_id(uint64_t id) { _id = id; }

  // operator

 private:
  IdbRect* _bounding_box;

  uint64_t _id;
};

}  // namespace idb
