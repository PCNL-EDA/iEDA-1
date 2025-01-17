/**
 * @File Name: dm_cts_config.cpp
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-04-15
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "dm_cts_config.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "json_parser.h"

namespace idm {
/**
 * @Brief : init data config from config file in config_path
 * @param  config_path config file path
 * @return true init config success
 * @return false init config failed
 */
bool CtsConfig::initConfig(string config_path)
{
  if (checkFilePath(config_path)) {
    _config_path = config_path;

    std::ifstream& config_stream = ieda::getInputFileStream(_config_path);

    {
      nlohmann::json json;
      config_stream >> json;

      _read_cts_data = ieda::getJsonData(json, {"file_path", "read_cts_data"});
      _write_cts_data = ieda::getJsonData(json, {"file_path", "write_cts_data"});
      _cts_data_path = ieda::getJsonData(json, {"file_path", "cts_data_file"});
    }

    ieda::closeFileStream(config_stream);

    return true;
  }
  return false;
}

/**
 * @Brief : check file exist in path
 * @param  path
 * @return true
 * @return false
 */
bool CtsConfig::checkFilePath(string path)
{
  FILE* file = fopen(path.c_str(), "r");
  if (file == nullptr) {
    std::cout << "[DataConfig error] : Can not open file = " << path << std::endl;

    return false;
  }

  fclose(file);

  return true;
}

}  // namespace idm
