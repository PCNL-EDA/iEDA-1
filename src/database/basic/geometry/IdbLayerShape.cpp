/**
 * @project		iDB
 * @file		IdbLayerShape.h
 * @date		25/05/2021
 * @version		0.1
 * @description


        Describe layer geometry shape.
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IdbLayerShape.h"

#include <limits.h>

#include <algorithm>

namespace idb {
IdbLayerShape::IdbLayerShape(IdbLayerShapeType type) : _type(type), _layer(nullptr)
{
}

IdbLayerShape::~IdbLayerShape()
{
  clear();
}
IdbLayerShape::IdbLayerShape(const IdbLayerShape& other)
{
  *this = other;
}
IdbLayerShape::IdbLayerShape(IdbLayerShape&& other)
{
  this->_layer = other._layer;
  this->_type = other._type;
  std::swap(this->_rect_list, other._rect_list);
}

void IdbLayerShape::clear()
{
  for (auto& rect : _rect_list) {
    if (rect != nullptr) {
      delete rect;
      rect = nullptr;
    }
  }
  _rect_list.clear();
  std::vector<IdbRect*>().swap(_rect_list);
}

IdbRect* IdbLayerShape::get_rect(uint index)
{
  if (index < 0 || index >= get_rect_list_num()) {
    return nullptr;
  } else {
    return _rect_list[index];
  }
}

IdbCoordinate<int32_t> IdbLayerShape::get_average_xy()
{
  int x = 0;
  int y = 0;
  for (IdbRect* rect : _rect_list) {
    x += rect->get_middle_point().get_x();
    y += rect->get_middle_point().get_y();
  }

  x = x / _rect_list.size();
  y = y / _rect_list.size();

  return IdbCoordinate<int32_t>(x, y);
}

void IdbLayerShape::add_rect(IdbRect* rect)
{
  _rect_list.emplace_back(rect);
}

void IdbLayerShape::add_rect(IdbRect rect)
{
  IdbRect* rect_new = new IdbRect(rect.get_low_x(), rect.get_low_y(), rect.get_high_x(), rect.get_high_y());
  add_rect(rect_new);
}

void IdbLayerShape::add_rect(int32_t ll_x, int32_t ll_y, int32_t ur_x, int32_t ur_y)
{
  IdbRect* rect = new IdbRect(ll_x, ll_y, ur_x, ur_y);
  add_rect(rect);
}

IdbRect IdbLayerShape::get_bounding_box()
{
  int32_t bounding_box_ll_x = INT_MAX;
  int32_t bounding_box_ll_y = INT_MAX;
  int32_t bounding_box_ur_x = INT_MIN;
  int32_t bounding_box_ur_y = INT_MIN;
  for (IdbRect* rect : _rect_list)

  {
    bounding_box_ll_x = std::min(bounding_box_ll_x, rect->get_low_x());
    bounding_box_ll_y = std::min(bounding_box_ll_y, rect->get_low_y());
    bounding_box_ur_x = std::max(bounding_box_ur_x, rect->get_high_x());
    bounding_box_ur_y = std::max(bounding_box_ur_y, rect->get_high_y());
  }

  return IdbRect(bounding_box_ll_x, bounding_box_ll_y, bounding_box_ur_x, bounding_box_ur_y);
}

void IdbLayerShape::moveToLocation(IdbCoordinate<int32_t>* coordinate)
{
  for (IdbRect* rect : _rect_list) {
    rect->moveByStep(coordinate->get_x(), coordinate->get_y());
  }
}

void IdbLayerShape::clone(IdbLayerShape& layer_shape)
{
  // (*this) = layer_shape;
  layer_shape = (*this);
}

IdbLayerShape& IdbLayerShape::operator=(const IdbLayerShape& other)
{
  if (this == &other) {
    return *this;
  }
  _type = other._type;
  _layer = other._layer;
  for (IdbRect* rect : other._rect_list) {
    IdbRect* rect_new = new IdbRect(rect);
    _rect_list.emplace_back(rect_new);
  }

  return (*this);
}

}  // namespace idb
