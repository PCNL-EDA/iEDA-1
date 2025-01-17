#pragma once
/**
 * @File Name: tcl_register.h
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-04-15
 *
 */
#include "./tcl_report_db/tcl_report_db.h"
#include "./tcl_report_evl/tcl_report_evl.h"
#include "./tcl_report_pr/tcl_report_route.h"
#include "ScriptEngine.hh"
#include "UserShell.hh"
#include "./tcl_report_pr/tcl_report_place.h"
using namespace ieda;
namespace tcl {

int registerCmdReportDb()
{
  registerTclCmd(CmdReportDbSummary, "report_db");

  return EXIT_SUCCESS;
}

int registerCmdReportEval()
{
  registerTclCmd(CmdReportWL, "report_wirelength");
  registerTclCmd(CmdReportCong, "report_congestion");
  // registerTclCmd(CmdReportDanglingNet, "report_dangling_net");

  return EXIT_SUCCESS;
}

int registerCmdReport()
{
  registerCmdReportDb();
  registerCmdReportEval();
  registerTclCmd(CmdReportRoute, "report_route");
  registerTclCmd(CmdReportPlaceDistro, "report_inst_distro");
  registerTclCmd(CmdReportPrefixedInst, "report_prefixed_instance");
  return EXIT_SUCCESS;
}

}  // namespace tcl