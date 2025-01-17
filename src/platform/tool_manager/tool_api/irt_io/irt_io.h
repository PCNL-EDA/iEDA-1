#pragma once
/**
 * @File Name: irt_io.h
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-17
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <any>
#include <map>
#include <string>
#include <vector>

#define RTIOInst (iplf::RTIO::getInst())

namespace iplf {

class RTIO
{
 public:
  static RTIO& getInst();
  static void delInst();
  // function

 private:
  static RTIO* _rt_io_instance;

  RTIO() = default;
  ~RTIO() = default;
};

}  // namespace iplf
