/**
 * @file CmdReadVerilog.cc
 * @author Wang Hao (harry0789@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-10-12
 */
#include "ShellCmd.hh"
#include "sta/Sta.hh"

namespace ista {
CmdReadVerilog::CmdReadVerilog(const char* cmd_name) : TclCmd(cmd_name) {
  auto* file_name_option = new TclStringOption("file_name", 1, nullptr);
  addOption(file_name_option);
}

unsigned CmdReadVerilog::check() {
  TclOption* file_name_option = getOptionOrArg("file_name");
  LOG_FATAL_IF(!file_name_option);
  return 1;
}

unsigned CmdReadVerilog::exec() {
  if (!check()) {
    return 0;
  }

  TclOption* file_name_option = getOptionOrArg("file_name");
  auto* verilog_file = file_name_option->getStringVal();

  Sta* ista = Sta::getOrCreateSta();

  ista->readVerilog(verilog_file);

  return 1;
}
}  // namespace ista