/*
 * @Author: S.J Chen
 * @Date: 2022-03-04 11:55:34
 * @LastEditTime: 2022-11-16 19:33:17
 * @LastEditors: sjchanson 13560469332@163.com
 * @Description:
 * @FilePath: /iEDA/src/iPL/src/util/utility.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IPL_UTIL_UTILITY_H
#define IPL_UTIL_UTILITY_H

#include <math.h>

#include <string>

#include "MultiTree.hh"
#include "data/Point.hh"

namespace ipl {

struct Utility
{
  Utility() = default;
  ~Utility() = default;

  int fastModulo(const int input, const int ceil);
  std::pair<int, int> obtainMinMaxIdx(int border_bottom, int interval, int inquiry_bottom, int inquiry_top);
  float getDistance(const std::vector<Point<int32_t>>& a, const std::vector<Point<int32_t>>& b);
  float getDistance(const std::vector<Point<float>>& a, const std::vector<Point<float>>& b);
};

// https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op
inline int Utility::fastModulo(const int input, const int ceil)
{
  return input >= ceil ? input % ceil : input;
}

inline std::pair<int, int> Utility::obtainMinMaxIdx(int border_bottom, int interval, int inquiry_bottom, int inquiry_top)
{
  int lower_idx = (inquiry_bottom - border_bottom) / interval;
  int upper_idx = (fastModulo((inquiry_top - border_bottom), interval) == 0) ? (inquiry_top - border_bottom) / interval
                                                                             : (inquiry_top - border_bottom) / interval + 1;
  return std::make_pair(lower_idx, upper_idx);
}

inline float Utility::getDistance(const std::vector<Point<int32_t>>& a, const std::vector<Point<int32_t>>& b)
{
  float sumDistance = 0.0f;
  for (size_t i = 0; i < a.size(); i++) {
    sumDistance += static_cast<float>(a[i].get_x() - b[i].get_x()) * static_cast<float>(a[i].get_x() - b[i].get_x());
    sumDistance += static_cast<float>(a[i].get_y() - b[i].get_y()) * static_cast<float>(a[i].get_y() - b[i].get_y());
  }

  return std::sqrt(sumDistance / (2.0 * a.size()));
}

inline float Utility::getDistance(const std::vector<Point<float>>& a, const std::vector<Point<float>>& b)
{
  float sumDistance = 0.0f;
  for (size_t i = 0; i < a.size(); i++) {
    sumDistance += (a[i].get_x() - b[i].get_x()) * (a[i].get_x() - b[i].get_x());
    sumDistance += (a[i].get_y() - b[i].get_y()) * (a[i].get_y() - b[i].get_y());
  }

  return std::sqrt(sumDistance / (2.0 * a.size()));
}

struct PointCMP
{
  bool operator()(const Point<int32_t>& lhs, const Point<int32_t>& rhs) const
  {
    if (lhs.get_x() == rhs.get_x()) {
      return lhs.get_y() < rhs.get_y();
    }

    return lhs.get_x() < rhs.get_x();
  }
};

}  // namespace ipl

#endif