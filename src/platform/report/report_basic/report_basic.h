#pragma once
/**
 * @File Name: report.h
 * @Brief :
 * @Author : Yell (12112088@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-11-07
 *
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ReportTable.hh"
#include "Str.hh"
namespace iplf {

class ReportBase
{
 public:
  explicit ReportBase(std::string report_name) : _report_name(std::move(report_name)) {}

  virtual ~ReportBase() = default;

  [[nodiscard]] const std::string& get_report_name() const { return _report_name; }

  bool add_table(std::shared_ptr<ieda::ReportTable> table);

  std::shared_ptr<ieda::ReportTable> get_table(int type);

  virtual std::string timestamp();
  virtual std::string seperator();
  virtual std::string title();
  std::vector<std::shared_ptr<ieda::ReportTable>>& get_table_list() { return _table_list; }

 private:
  std::string _report_name;
  std::vector<std::shared_ptr<ieda::ReportTable>> _table_list;
};

std::ostream& operator<<(std::ostream& ost, ReportBase& report);
}  // namespace iplf