#pragma once

#include <ostream>
#include <string>

#include "IdbDesign.h"
#include "IdbPins.h"
#include "ReportTable.hh"
#include "report_basic.h"

namespace iplf {

enum class ReportDBType : int
{
  kNone = 0,
  kTitle,
  kSummary,
  kkSummaryLef,
  kSummaryInstance,
  kSummaryNet,
  kSummaryLayer,
  kSummaryPin,
  kInstance,
  kNet,
  kSpecialNet,
  kInstancePinList,
  kMax
};

class ReportDB : public ReportBase
{
  struct SummaryLayerValue
  {
    std::string layer_name;
    int32_t layer_order;
    uint64_t wire_len;
    uint64_t seg_num;
    uint64_t wire_num;
    uint64_t via_num;
    uint64_t patch_num;
  };

  const int max_num = 34;
  const int max_fanout = 32;

 public:
  explicit ReportDB(const std::string& report_name) : ReportBase(report_name) {}

  std::string title() override;

  std::shared_ptr<ieda::ReportTable> createSummaryTable();
  std::shared_ptr<ieda::ReportTable> createSummaryInstances();
  std::shared_ptr<ieda::ReportTable> createSummaryNets();
  std::shared_ptr<ieda::ReportTable> createSummaryLayers();
  std::shared_ptr<ieda::ReportTable> createSummaryPins();
};

class ReportDesign : public ReportBase
{
 public:
  explicit ReportDesign(const std::string& report_name) : ReportBase(report_name) {}

  std::string title() override;
  std::shared_ptr<ieda::ReportTable> createInstanceTable(const std::string& inst_name);
  std::shared_ptr<ieda::ReportTable> createInstanceTable(idb::IdbInstance* inst);
  std::shared_ptr<ieda::ReportTable> createInstancePinTable(const std::string& inst_name);
  std::shared_ptr<ieda::ReportTable> createInstancePinTable(idb::IdbInstance* inst);

  std::shared_ptr<ieda::ReportTable> createNetTable(const std::string& net_name);
  std::shared_ptr<ieda::ReportTable> createNetTable(idb::IdbNet* net);

 private:
};

class ReportDanglingNet
{
 public:
  explicit ReportDanglingNet() = default;
  [[nodiscard]] int32_t get_count() const { return _count; }
  void count() { ++_count; }
  void resetCount() { _count = 0; }

 private:
  int32_t _count = 0;
};

std::ostream& operator<<(std::ostream& os, ReportDanglingNet& report);

}  // namespace iplf