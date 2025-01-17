
#pragma once

#include <map>
#include <string>
#include <vector>

#include "FPInst.hh"
#include "FPNet.hh"
#include "FPPin.hh"

namespace ipl::imp {

class FPDesign
{
 public:
  FPDesign() = default;
  ~FPDesign();

  // getter
  string get_design_name() const { return _design_name; }
  vector<FPInst*> get_std_cell_list() const { return _std_cell_list; }
  vector<FPInst*> get_macro_list() const { return _macro_list; }
  vector<FPNet*> get_net_list() const { return _net_list; }
  vector<FPPin*> get_pin_list() const { return _pin_list; }

  // setter
  void set_design_name(string name) { _design_name = name; }
  void add_std_cell(FPInst* inst) { _std_cell_list.emplace_back(inst); }
  void add_macro(FPInst* macro) { _macro_list.emplace_back(macro); }
  void add_net(FPNet* net) { _net_list.emplace_back(net); }
  void add_pin(FPPin* pin) { _pin_list.emplace_back(pin); }

  // function

 private:
  string _design_name;
  vector<FPInst*> _std_cell_list;
  vector<FPInst*> _macro_list;
  vector<FPNet*> _net_list;
  vector<FPPin*> _pin_list;
};

inline FPDesign::~FPDesign()
{
  for (FPInst* inst : _std_cell_list) {
    delete inst;
  }
  _std_cell_list.clear();
  _macro_list.clear();
  for (FPNet* net : _net_list) {
    delete net;
  }
  _net_list.clear();
  for (FPPin* pin : _pin_list) {
    delete pin;
  }
  _pin_list.clear();
}

}  // namespace ipl::imp