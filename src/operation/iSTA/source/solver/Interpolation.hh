/**
 * @file Interpolation.hh
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The utility function of ista.
 * @version 0.1
 * @date 2021-06-29
 *
 *
 */

#pragma once

#include <cstdlib>

#include "include/Type.hh"

namespace ista {

double LinearInterpolate(double x1, double x2, double y1, double y2, double x);
double BilinearInterpolation(double q11, double q12, double q21, double q22,
                             double x1, double x2, double y1, double y2,
                             double x, double y);
}  // namespace ista
