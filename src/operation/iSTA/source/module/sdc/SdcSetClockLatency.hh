/**
 * @file SdcSetClockUncertainty.hh
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The sdc set_clock_uncertainty class.
 * @version 0.1
 * @date 2021-10-20
 */
#pragma once
#include <set>

#include "SdcCommand.hh"
#include "netlist/DesignObject.hh"

namespace ista {

/**
 * @brief The constrain of clock latency.
 *
 */
class SdcSetClockLatency : public SdcCommandObj {
 public:
  explicit SdcSetClockLatency(double delay_value);
  ~SdcSetClockLatency() override = default;

  void set_rise() { _rise = 1; }
  void set_fall() { _fall = 1; }
  void set_max() { _max = 1; }
  void set_min() { _min = 1; }
  void set_early() { _early = 1; }
  void set_late() { _late = 1; }

  [[nodiscard]] unsigned isRise() const { return _rise; }
  [[nodiscard]] unsigned isFall() const { return _fall; }
  [[nodiscard]] unsigned isMax() const { return _max; }
  [[nodiscard]] unsigned isMin() const { return _min; }
  [[nodiscard]] unsigned isEarly() const { return _early; }
  [[nodiscard]] unsigned isLate() const { return _late; }

  void set_objs(std::set<DesignObject*>&& objs) { _objs = std::move(objs); }
  auto& get_objs() { return _objs; }

  [[nodiscard]] double get_delay_value() const { return _delay_value; }

 private:
  unsigned _rise : 1;
  unsigned _fall : 1;
  unsigned _max : 1;
  unsigned _min : 1;
  unsigned _early : 1;
  unsigned _late : 1;
  unsigned _reserved : 26;

  double _delay_value;

  std::set<DesignObject*> _objs;  //!< The objects.
};

}  // namespace ista
