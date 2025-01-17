#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "NoConfig.h"

namespace iplf {

#define iNOInst (NoIO::getInstance())
class NoIO
{
 public:
  static NoIO* getInstance()
  {
    if (!_instance) {
      _instance = new NoIO;
    }
    return _instance;
  }

  /// io
  bool runNOFixFanout(std::string config = "");

 private:
  static NoIO* _instance;

  NoIO() {}
  ~NoIO() = default;

  void resetConfig(ino::NoConfig* no_config);
};

}  // namespace iplf
