/**
 * @file Cmd.h
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The sdc cmd class.
 * @version 0.1
 * @date 2021-03-02
 */

#pragma once

#include "netlist/DesignObject.hh"
#include "sdc/SdcClock.hh"
#include "sdc/SdcCollection.hh"
#include "sdc/SdcConstrain.hh"
#include "sdc/SdcTimingDRC.hh"
#include "sta/Sta.hh"
#include "tcl/ScriptEngine.hh"

namespace ista {

using ieda::ScriptEngine;
using ieda::TclCmd;
using ieda::TclCmds;
using ieda::TclDoubleListOption;
using ieda::TclDoubleOption;
using ieda::TclEncodeResult;
using ieda::TclIntListOption;
using ieda::TclIntOption;
using ieda::TclOption;
using ieda::TclStringListListOption;
using ieda::TclStringListOption;
using ieda::TclStringOption;
using ieda::TclSwitchOption;

/**
 * @brief create_clock command.
 *
 */
class CmdCreateClock : public TclCmd {
 public:
  explicit CmdCreateClock(const char* cmd_name);
  ~CmdCreateClock() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief create_generated_clock command.
 *
 */
class CmdCreateGeneratedClock : public TclCmd {
 public:
  explicit CmdCreateGeneratedClock(const char* cmd_name);
  ~CmdCreateGeneratedClock() override = default;

  unsigned check() override;
  unsigned exec() override;

 private:
  void set_source_sdc_clock(std::vector<const char*>);
  void set_master_clock(std::vector<const char*>);
  void set_generate_clock(std::vector<const char*>);
  void set_period_and_edges(std::vector<const char*>);
  void set_duty_cycle(std::vector<const char*>);
  void set_edges_shift_and_invert(std::vector<const char*>);
  void set_source_objects(std::vector<const char*>);
  void set_add(std::vector<const char*>);
  void set_comment(std::vector<const char*>);
  void init_generate_clock_by_source();

  double _duty_cycle = 0;
  SdcClock* _source_sdc_clock = nullptr;
  SdcGenerateCLock* _the_generate_clock = nullptr;

  SdcConstrain* _the_constrain = nullptr;
};

/**
 * @brief set_input_transition command.
 *
 */
class CmdSetInputTransition : public TclCmd {
 public:
  explicit CmdSetInputTransition(const char* cmd_name);
  ~CmdSetInputTransition() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_load command.
 *
 */
class CmdSetLoad : public TclCmd {
 public:
  explicit CmdSetLoad(const char* cmd_name);
  ~CmdSetLoad() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_input_delay command.
 *
 */
class CmdSetInputDelay : public TclCmd {
 public:
  explicit CmdSetInputDelay(const char* cmd_name);
  ~CmdSetInputDelay() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_output_delay command.
 *
 */
class CmdSetOutputDelay : public TclCmd {
 public:
  explicit CmdSetOutputDelay(const char* cmd_name);
  ~CmdSetOutputDelay() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_clock_uncertainty command.
 *
 */
class CmdSetClockUncertainty : public TclCmd {
 public:
  explicit CmdSetClockUncertainty(const char* cmd_name);
  ~CmdSetClockUncertainty() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_clock_latency command.
 *
 */
class CmdSetClockLatency : public TclCmd {
 public:
  explicit CmdSetClockLatency(const char* cmd_name);
  ~CmdSetClockLatency() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_timing_derate command.
 *
 */
class CmdSetTimingDerate : public TclCmd {
 public:
  explicit CmdSetTimingDerate(const char* cmd_name);
  ~CmdSetTimingDerate() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_max_fanout
 *
 * Sets maximum fanout for input ports or designs
 *
 */
class CmdSetMaxFanout : public TclCmd {
 public:
  explicit CmdSetMaxFanout(const char* cmd_name);
  ~CmdSetMaxFanout() override = default;

  unsigned check() override;
  unsigned exec() override;

 private:
  void setObjectLists(SetMaxFanout* set_max_fanout);
  void addTimingDRC2Constrain(SetMaxFanout* set_max_fanout);
};

/**
 * @brief set_max_transition
 *
 * Sets maximum transition for pins, ports, clocks, or designs with respect to
 * the main library trip-points.
 *
 */
class CmdSetMaxTransition : public TclCmd {
 public:
  explicit CmdSetMaxTransition(const char* cmd_name);
  ~CmdSetMaxTransition() override = default;

  unsigned check() override;
  unsigned exec() override;

 private:
  void setClockDataPath(SetMaxTransition* set_max_transition);
  void setRiseFall(SetMaxTransition* set_max_transition);
  void setObjectLists(SetMaxTransition* set_max_transition);
  void addTimingDRC2Constrain(SetMaxTransition* set_max_transition);
};

/**
 * @brief set_max_capacitance
 *
 * Sets maximum capacitance for pins, ports, clocks or designs
 *
 */
class CmdSetMaxCapacitance : public TclCmd {
 public:
  explicit CmdSetMaxCapacitance(const char* cmd_name);
  ~CmdSetMaxCapacitance() override = default;

  unsigned check() override;
  unsigned exec() override;

 private:
  void setClockDataPath(SetMaxCapacitance* set_max_cap);
  void setRiseFall(SetMaxCapacitance* set_max_cap);
  void setObjectLists(SetMaxCapacitance* set_max_cap);
  void addTimingDRC2Constrain(SetMaxCapacitance* set_max_cap);
};

/**
 * @brief current_design get the current design netlist.
 */
class CmdCurrentDesign : public TclCmd {
 public:
  explicit CmdCurrentDesign(const char* cmd_name);
  ~CmdCurrentDesign() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief get_clocks get the matched clock.
 *
 */
class CmdGetClocks : public TclCmd {
 public:
  explicit CmdGetClocks(const char* cmd_name);
  ~CmdGetClocks() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief get_ports get the matched port.
 *
 */
class CmdGetPorts : public TclCmd {
 public:
  explicit CmdGetPorts(const char* cmd_name);
  ~CmdGetPorts() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief get_pins get the matched pins.
 *
 */
class CmdGetPins : public TclCmd {
 public:
  explicit CmdGetPins(const char* cmd_name);
  ~CmdGetPins() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_propagated_clock, specifies the clock to be propagated.
 *
 */
class CmdSetPropagatedClock : public TclCmd {
 public:
  explicit CmdSetPropagatedClock(const char* cmd_name);
  ~CmdSetPropagatedClock() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief all_clocks, get all clocks.
 *
 */
class CmdAllClocks : public TclCmd {
 public:
  explicit CmdAllClocks(const char* cmd_name);
  ~CmdAllClocks() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_clock_groups, set clock relationship.
 *
 */
class CmdSetClockGroups : public TclCmd {
 public:
  explicit CmdSetClockGroups(const char* cmd_name);
  ~CmdSetClockGroups() override = default;

  unsigned check() override;
  unsigned exec() override;
};

/**
 * @brief set_multicycle_path cmd.
 *
 */
class CmdSetMulticyclePath : public TclCmd {
 public:
  explicit CmdSetMulticyclePath(const char* cmd_name);
  ~CmdSetMulticyclePath() override = default;

  unsigned check() override;
  unsigned exec() override;
};

}  // namespace ista
