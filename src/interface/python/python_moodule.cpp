#include "ScriptEngine.hh"
#include "py_register_config.h"
#include "py_register_flow.h"
#include "py_register_icts.h"
#include "py_register_idb.h"
#include "py_register_idrc.h"
#include "py_register_ifp.h"
#include "py_register_ino.h"
#include "py_register_inst.h"
#include "py_register_ipdn.h"
#include "py_register_ipl.h"
// #include "py_register_irt.h"
#include "py_register_ista.h"
#include "py_register_ito.h"
#include "python_module.h"
#include "py_register_report.h"
namespace python_interface {

PYBIND11_MODULE(ieda_py, m)
{
  register_config(m);
  register_flow(m);
  register_icts(m);
  register_idb(m);
  register_idb_op(m);
  register_idrc(m);
  register_ifp(m);
  register_ino(m);
  register_inst(m);
  register_ipdn(m);
  register_ipl(m);
  // register_irt(m);
  register_ista(m);
  register_ito(m);
  register_report(m);
}

}  // namespace python_interface