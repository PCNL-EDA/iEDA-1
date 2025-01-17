/**
 * @file SdcSetClockUncertainty.hh
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The sdc set_clock_uncertainty class.
 * @version 0.1
 * @date 2021-10-20
 */
#pragma once
#include <set>

#include "SdcCollection.hh"
#include "SdcCommand.hh"
#include "netlist/DesignObject.hh"

namespace ista {

/**
 * @brief The constrain of clock uncertainty.
 *
 */
class SdcSetClockUncertainty : public SdcCommandObj {
 public:
  explicit SdcSetClockUncertainty(double uncertainty_value);
  ~SdcSetClockUncertainty() override = default;

  void set_rise(bool is_set) { _rise = is_set; }
  void set_fall(bool is_set) { _fall = is_set; }
  void set_setup(bool is_set) { _setup = is_set; }
  void set_hold(bool is_set) { _hold = is_set; }

  [[nodiscard]] unsigned isRise() const { return _rise; }
  [[nodiscard]] unsigned isFall() const { return _fall; }
  [[nodiscard]] unsigned isSetup() const { return _setup; }
  [[nodiscard]] unsigned isHold() const { return _hold; }

  void set_objs(std::set<SdcCollectionObj>&& objs) { _objs = std::move(objs); }
  auto& get_objs() { return _objs; }

  [[nodiscard]] double get_uncertainty_value() const {
    return _uncertainty_value;
  }
  [[nodiscard]] auto getUncertaintyValueFs() const {
    return NS_TO_FS(_uncertainty_value);
  }

 private:
  unsigned _rise : 1;
  unsigned _fall : 1;
  unsigned _setup : 1;
  unsigned _hold : 1;
  unsigned _reserved : 28;

  double _uncertainty_value;

  std::set<SdcCollectionObj> _objs;  //!< The objects.
};

}  // namespace ista
