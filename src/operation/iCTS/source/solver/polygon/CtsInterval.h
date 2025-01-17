#pragma once

#include <algorithm>

namespace icts {

template <typename T>
class CtsInterval {
 public:
  typedef T coord_t;
  typedef coord_t coordinate_type;

  CtsInterval() : _coords() {}

  CtsInterval(coord_t low, coord_t high) {
    if (low > high) {
      std::swap(low, high);
    }
    _coords[gtl::LOW] = low;
    _coords[gtl::HIGH] = high;
  }

  CtsInterval(const CtsInterval &that) {
    _coords[0] = that._coords[0];
    _coords[1] = that._coords[1];
  }
  CtsInterval &operator=(const CtsInterval &that) {
    _coords[0] = that._coords[0];
    _coords[1] = that._coords[1];
    return *this;
  }
  coord_t get(gtl::direction_1d dir) const { return _coords[dir.to_int()]; }

  void set(gtl::direction_1d dir, coord_t value) {
    _coords[dir.to_int()] = value;
  }

  coord_t low() const { return _coords[gtl::LOW]; }
  CtsInterval &low(coord_t value) {
    _coords[gtl::LOW] = value;
    return *this;
  }
  coord_t high() const { return _coords[gtl::HIGH]; }
  void high(coord_t value) { _coords[gtl::HIGH] = value; }

  CtsInterval operator&(const CtsInterval &that) {
    coord_t low = std::max(this->low(), that.low());
    coord_t high = std::min(this->high(), that.high());
    return CtsInterval(low, high);
  }

  bool operator==(const CtsInterval &that) const {
    return low() == that.low() && high() == that.high();
  }
  bool operator!=(const CtsInterval &that) const { return !(*this == that); }
  bool operator<(const CtsInterval &that) const {
    return low() < that.low() || (low() == that.low() && high() < that.high());
  }
  bool operator>(const CtsInterval &that) const { return that < *this; }
  bool operator<=(const CtsInterval &that) const { return !(that < *this); }
  bool operator>=(const CtsInterval &that) const { return !(*this < that); }

 private:
  coord_t _coords[2];
};

}  // namespace icts