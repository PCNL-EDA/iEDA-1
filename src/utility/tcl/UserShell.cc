/**
 * @file UserShell.cc
 * @author Wang Hao (harry0789@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-10-22
 */
#include "UserShell.hh"

namespace ieda {

int UserShell::userMain(const char* file_path)
{
  int argc = 1;
  char** argv = new char*[2];
  argv[0] = const_cast<char*>("UserShell\n");

  if (file_path) {
    argc = 2;
    if (argv == nullptr) {
      std::cerr << "fail to malloc memery\n";
      return EXIT_FAILURE;
    }
    argv[1] = const_cast<char*>(file_path);
  }

  auto* script_engine = ScriptEngine::getOrCreateInstance();
  // note: Class member function has a implicit arguement "this".
  // So you can't pass a member function as callback unless it is static.
  Tcl_MainEx(argc, argv, initUserSetting, script_engine->get_interp());

  delete[] argv;
  return EXIT_SUCCESS;
}

void UserShell::displayHelp()
{
  std::cerr << "\033[49;32m"
               "Enter \033[1mexit\033[0m\033[49;32m to quit."
               "\033[0m\n";
}

UserShell* UserShell::getShell()
{
  static UserShell shell;
  return &shell;
}

static int defaultInit()
{
  std::cerr << "notice: no user defined initialized function\n";
  return EXIT_SUCCESS;
}

int (*UserShell::_user_init)() = defaultInit;

void UserShell::set_init_func(int (*callback_func)())
{
  _user_init = callback_func;
}
}  // namespace ieda