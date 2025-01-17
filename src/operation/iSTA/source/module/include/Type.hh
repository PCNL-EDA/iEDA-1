/**
 * @file Type.h
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief
 * @version 0.1
 * @date 2021-02-22
 */

#pragma once

#include <cmath>
#include <utility>

namespace ista {

enum class AnalysisMode : int { kMax = 1, kMin = 2, kMaxMin = 3 };
enum class TransType : int { kRise = 1, kFall = 2, kRiseFall = 3 };

#define IS_MAX(analysis_mode) (static_cast<int>(analysis_mode) & 0b01)
#define IS_MIN(analysis_mode) (static_cast<int>(analysis_mode) & 0b10)

#define IS_RISE(trans_type) (static_cast<int>(trans_type) & 0b01)
#define IS_FALL(trans_type) (static_cast<int>(trans_type) & 0b10)

#define FLIP_TRANS(type) \
  (((type) == TransType::kRise) ? TransType::kFall : TransType::kRise)

#define MODE_TRANS_SPLIT 4
#define MODE_SPLIT 2
#define TRANS_SPLIT 2

static constexpr int g_ns2ps = 1000;
static constexpr int g_ns2fs = 1000000;
static constexpr int g_ps2fs = 1000;
static constexpr int g_pf2ff = 1000;
static constexpr double g_pf2f = 1e-12;

static constexpr std::initializer_list<std::pair<AnalysisMode, TransType>>
    g_split_trans = {{AnalysisMode::kMax, TransType::kRise},
                     {AnalysisMode::kMax, TransType::kFall},
                     {AnalysisMode::kMin, TransType::kRise},
                     {AnalysisMode::kMin, TransType::kFall}};

enum class ModeTransIndex : int {
  kMaxRise = 0,
  kMaxFall = 1,
  kMinRise = 2,
  kMinFall = 3
};

using ModeTransPair = std::pair<AnalysisMode, TransType>;

#define FOREACH_MODE_TRANS(mode, trans) for (auto [mode, trans] : g_split_trans)

#define NS_TO_FS(delay) ((delay) * static_cast<int64_t>(g_ns2fs))
#define FS_TO_NS(delay) ((delay) / static_cast<double>(g_ns2fs))
#define NS_TO_PS(delay) ((delay) * static_cast<int64_t>(g_ns2ps))
#define PS_TO_NS(delay) ((delay) / static_cast<double>(g_ns2ps))
#define PS_TO_FS(delay) ((delay) * static_cast<int64_t>(g_ps2fs))
#define FS_TO_PS(delay) ((delay) / static_cast<double>(g_ps2fs))

#define PF_TO_FF(cap) (static_cast<int>(std::ceil((cap)*g_pf2ff)))
#define FF_TO_PF(cap) ((cap) / static_cast<double>(g_pf2ff))

#define PF_TO_F(cap) ((cap)*g_pf2f)
#define F_TO_PF(cap) ((cap) / g_pf2f)

enum class DelayCalcMethod : int { kElmore = 0, kArnoldi = 1 };

enum class CapacitiveUnit { kPF = 0, kFF = 1 };
enum class ResistanceUnit { kOHM = 0, kkOHM = 1 };

inline double ConvertCapUnit(CapacitiveUnit src_unit, CapacitiveUnit snk_unit,
                             double cap) {
  double converted_cap = cap;
  if (src_unit != snk_unit) {
    if ((src_unit == CapacitiveUnit::kFF) &&
        (snk_unit == CapacitiveUnit::kPF)) {
      converted_cap = FF_TO_PF(cap);
    } else if ((src_unit == CapacitiveUnit::kPF) &&
               (snk_unit == CapacitiveUnit::kFF)) {
      converted_cap = PF_TO_FF(cap);
    }
  }

  return converted_cap;
}

constexpr double double_precesion = 1e-15;
constexpr bool IsDoubleEqual(double data1, double data2,
                             double eplison = double_precesion) {
  return std::abs(data1 - data2) < eplison;
}

// helper type for the std visitor
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace ista
