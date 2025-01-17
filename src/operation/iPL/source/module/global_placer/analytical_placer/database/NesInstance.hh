/*
 * @Author: S.J Chen
 * @Date: 2022-03-09 21:30:57
 * @LastEditTime: 2022-10-27 19:33:27
 * @LastEditors: sjchanson 13560469332@163.com
 * @Description:
 * @FilePath: /iEDA/src/iPL/src/operator/global_placer/NesterovPlace/database/NesInstance.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IPL_OPERATOR_NESTEROV_PLACE_DATABASE_NESINSTANCE_H
#define IPL_OPERATOR_NESTEROV_PLACE_DATABASE_NESINSTANCE_H

#include <string>
#include <vector>

#include "NesPin.hh"
#include "data/Rectangle.hh"

namespace ipl {

class NesInstance
{
 public:
  NesInstance() = delete;
  explicit NesInstance(std::string name);
  NesInstance(const NesInstance&) = delete;
  NesInstance(NesInstance&&) = delete;
  ~NesInstance() = default;

  NesInstance& operator=(const NesInstance&) = delete;
  NesInstance& operator=(NesInstance&&) = delete;

  // getter.
  std::string get_name() const { return _name; }
  float get_density_scale() const { return _density_scale; }

  Rectangle<int32_t> get_origin_shape() const { return _origin_shape; }
  Rectangle<int32_t> get_density_shape() const { return _density_shape; }
  Point<int32_t> get_density_coordi() const { return _density_shape.get_lower_left(); }
  Point<int32_t> get_density_center_coordi() const { return _density_shape.get_center(); }

  std::vector<NesPin*> get_nPin_list() const { return _nPin_list; }

  bool isFixed() const { return _is_fixed == 1; }
  bool isFiller() const { return _is_filler == 1; }
  bool isMacro() const { return _is_macro == 1; }

  // setter.
  void set_density_scale(float scale) { _density_scale = scale; }

  void set_origin_shape(Rectangle<int32_t> shape) { _origin_shape = std::move(shape); }
  void set_density_shape(Rectangle<int32_t> shape) { _density_shape = std::move(shape); }

  void add_nPin(NesPin* nPin) { _nPin_list.push_back(nPin); }

  void set_fixed() { _is_fixed = 1; }
  void set_filler() { _is_filler = 1; }
  void set_macro() { _is_macro = 1; }

  // function.
  void updateDensityLocation(Point<int32_t> coordi);
  void updateDensityCenterLocation(Point<int32_t>& center_coordi);
  void changeSize(int width, int height);

 private:
  std::string _name;
  float _density_scale;

  Rectangle<int32_t> _origin_shape;
  Rectangle<int32_t> _density_shape;

  std::vector<NesPin*> _nPin_list;

  unsigned char _is_fixed : 1;
  unsigned char _is_filler : 1;
  // need to be stored for MS place.
  unsigned char _is_macro : 1;

  void updateNesPinListLocation();
};
inline NesInstance::NesInstance(std::string name) : _name(std::move(name)), _density_scale(1.0F), _is_fixed(0), _is_filler(0), _is_macro(0)
{
}

inline void NesInstance::updateDensityLocation(Point<int32_t> coordi)
{
  _density_shape.set_rectangle(coordi.get_x(), coordi.get_y(), coordi.get_x() + _density_shape.get_width(),
                               coordi.get_y() + _density_shape.get_height());

  updateNesPinListLocation();
}

inline void NesInstance::updateDensityCenterLocation(Point<int32_t>& center_coordi)
{
  _density_shape.set_center(center_coordi.get_x(), center_coordi.get_y());

  updateNesPinListLocation();
}

inline void NesInstance::updateNesPinListLocation()
{
  for (auto* nPin : _nPin_list) {
    Point<int32_t> inst_center = this->get_density_center_coordi();
    Point<int32_t> nPin_offset = nPin->get_offset_coordi();

    nPin->set_center_coordi(Point<int32_t>(inst_center.get_x() + nPin_offset.get_x(), inst_center.get_y() + nPin_offset.get_y()));
  }
}

// change size and preserve center coodinates
inline void NesInstance::changeSize(int width, int height)
{
  const int center_x = _origin_shape.get_center().get_x();
  const int center_y = _origin_shape.get_center().get_y();

  int lx = center_x - width / 2;
  int ly = center_y - height / 2;
  int ux = center_x + width / 2;
  int uy = center_y + height / 2;

  set_origin_shape(Rectangle<int32_t>(lx, ly, ux, uy));
}

}  // namespace ipl

#endif  // SRC_OPERATION_IPL_SOURCE_MODULE_GLOBAL_PLACER_ANALYTICAL_PLACER_DATABASE_NESINSTANCE_HH_
