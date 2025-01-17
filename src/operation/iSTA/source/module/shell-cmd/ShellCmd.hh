/**
 * @file SellCmd.hh
 * @author Wang Hao (harryw0789@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-09-28
 */
#pragma once

#include "netlist/DesignObject.hh"
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
 * @brief set the design workspace.
 *
 */
class CmdSetDesignWorkSpace : public TclCmd {
 public:
  explicit CmdSetDesignWorkSpace(const char* cmd_name);
  ~CmdSetDesignWorkSpace() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief Reads in one or more (will support) Verilog files.
 *
 */
class CmdReadVerilog : public TclCmd {
 public:
  explicit CmdReadVerilog(const char* cmd_name);
  ~CmdReadVerilog() override = default;

  unsigned check();
  unsigned exec();
};

class CmdTESTSLL : public TclCmd {
 public:
  explicit CmdTESTSLL(const char* cmd_name);
  ~CmdTESTSLL() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief Reads in one Liberty files.
 *
 */
class CmdReadLiberty : public TclCmd {
 public:
  explicit CmdReadLiberty(const char* cmd_name);
  ~CmdReadLiberty() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief Specifies the name of the design to be linked; the default is the
 * current design.
 *
 */
class CmdLinkDesign : public TclCmd {
 public:
  explicit CmdLinkDesign(const char* cmd_name);
  ~CmdLinkDesign() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief read_spef command.
 *
 */
class CmdReadSpef : public TclCmd {
 public:
  explicit CmdReadSpef(const char* cmd_name);
  ~CmdReadSpef() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief Reads in a script in Synopsys Design Constraints (SDC) format.
 *
 */
class CmdReadSdc : public TclCmd {
 public:
  explicit CmdReadSdc(const char* cmd_name);
  ~CmdReadSdc() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief report_checks command reports paths in the design.
 *
 */
class CmdReportTiming : public TclCmd {
 public:
  explicit CmdReportTiming(const char* cmd_name);
  ~CmdReportTiming() override = default;

  unsigned check();
  unsigned exec();
};

/**
 * @brief report_check_types command reports the slack for each type of timing
 * and design rule constraint. The keyword options allow a subset of the
 * constraint types to be reported.
 *
 */
class CmdReportConstraint : public TclCmd {
 public:
  explicit CmdReportConstraint(const char* cmd_name);
  ~CmdReportConstraint() override = default;

  unsigned check();
  unsigned exec();
};

}  // namespace ista
