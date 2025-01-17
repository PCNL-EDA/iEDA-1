/**
 * @file StaReport.h
 * @author simin tao (taosm@pcl.ac.cn)
 * @brief The class of report timing path.
 * @version 0.1
 * @date 2021-04-23
 */

#pragma once

#include <memory>

#include "StaPathData.hh"
#include "report/ReportTable.hh"

namespace ista {

class Sta;

/**
 * @brief The timing report table wrapper, the detailed table use the third
 * party fort table.
 *
 */
class StaReportTable : public ieda::ReportTable {
 public:
  explicit StaReportTable(const char* table_name)
      : ieda::ReportTable(table_name) {}
  ~StaReportTable() override = default;

  using Base = ieda::ReportTable;

  using Base::operator<<;
  using Base::operator[];

  using Base::writeRow;
  using Base::writeRowFromIterator;
};

/**
 * @brief The report path function class.
 *
 */
class StaReportPathSummary {
 public:
  StaReportPathSummary(const char* rpt_file_name, AnalysisMode analysis_mode,
                       unsigned n_worst = 3);
  virtual ~StaReportPathSummary();

  [[nodiscard]] unsigned get_n_worst() const { return _n_worst; }
  [[nodiscard]] unsigned get_significant_digits() const {
    return _significant_digits;
  }
  void set_significant_digits(unsigned significant_digits) {
    _significant_digits = significant_digits;
  }

  static std::unique_ptr<StaReportTable> createReportTable(
      const char* tbl_name);

  virtual unsigned operator()(StaSeqPathData* seq_path_data);
  virtual unsigned operator()(StaSeqPathGroup* seq_path_group);

 private:
  const char* _rpt_file_name;        //!< The report file name.
  AnalysisMode _analysis_mode;       //!< The max/min analysis mode.
  unsigned _n_worst;                 //!< The top n path num.
  unsigned _significant_digits = 3;  //!< The significant digits.
};

/**
 * @brief The report clock TNS information class.
 *
 */
class StaReportClockTNS : public StaReportPathSummary {
 public:
  StaReportClockTNS(const char* rpt_file_name, AnalysisMode analysis_mode,
                    unsigned n_worst);
  ~StaReportClockTNS() override = default;

  static std::unique_ptr<StaReportTable> createReportTable(
      const char* tbl_name);
  unsigned operator()(StaSeqPathData* seq_path_data);
  unsigned operator()(StaSeqPathGroup* seq_path_group) override {
    return StaReportPathSummary::operator()(seq_path_group);
  }
};

/**
 * @brief The report path detail information class.
 *
 */
class StaReportPathDetail : public StaReportPathSummary {
 public:
  StaReportPathDetail(const char* rpt_file_name, AnalysisMode analysis_mode,
                      unsigned n_worst, bool is_derate);
  ~StaReportPathDetail() override = default;

  [[nodiscard]] bool get_is_derate() const { return _is_derate; }

  static std::unique_ptr<StaReportTable> createReportTable(const char* tbl_name,
                                                           bool is_derate);

  unsigned operator()(StaSeqPathData* seq_path_data) override;
  unsigned operator()(StaSeqPathGroup* seq_path_group) override {
    return StaReportPathSummary::operator()(seq_path_group);
  }

 private:
  bool _is_derate;
};

/**
 * @brief The report path dump inner data information.
 *
 */
class StaReportPathDump : public StaReportPathSummary {
 public:
  StaReportPathDump(const char* rpt_file_name, AnalysisMode analysis_mode,
                    unsigned n_worst);
  ~StaReportPathDump() override = default;

  unsigned operator()(StaSeqPathData* seq_path_data) override;
  unsigned operator()(StaSeqPathGroup* seq_path_group) override {
    return StaReportPathSummary::operator()(seq_path_group);
  }
};

/**
 * @brief The DRV trans report.
 *
 */
class StaReportTrans {
 public:
  StaReportTrans(const char* rpt_file_name, AnalysisMode analysis_mode,
                 unsigned n_worst);
  ~StaReportTrans();

  std::unique_ptr<StaReportTable> createReportTable(const char* tbl_name);
  unsigned operator()(Sta* ista);

 private:
  const char* _rpt_file_name;   //!< The report file name.
  AnalysisMode _analysis_mode;  //!< The max/min analysis mode.
  unsigned _n_worst;            //!< The top n path num.
};

/**
 * @brief The DRV trans report.
 *
 */
class StaReportCap {
 public:
  StaReportCap(const char* rpt_file_name, AnalysisMode analysis_mode,
               unsigned n_worst, bool is_clock_cap);
  ~StaReportCap();

  std::unique_ptr<StaReportTable> createReportTable(const char* tbl_name);
  unsigned operator()(Sta* ista);
  bool get_is_clock_cap() const { return _is_clock_cap; }

 private:
  const char* _rpt_file_name;   //!< The report file name.
  AnalysisMode _analysis_mode;  //!< The max/min analysis mode.
  unsigned _n_worst;            //!< The top n path num.
  bool _is_clock_cap;  //!< The flag decide whether the report with sequential
                       //!< and logic or the report with only sequential.
};

/**
 * @brief The DRV trans report.
 *
 */
class StaReportFanout {
 public:
  StaReportFanout(const char* rpt_file_name, AnalysisMode analysis_mode,
                  unsigned n_worst);
  ~StaReportFanout();

  std::unique_ptr<StaReportTable> createReportTable(const char* tbl_name);
  unsigned operator()(Sta* ista);

 private:
  const char* _rpt_file_name;   //!< The report file name.
  AnalysisMode _analysis_mode;  //!< The max/min analysis mode.
  unsigned _n_worst;            //!< The top n path num.
};

/**
 * @brief The skew summary report.
 *
 */
class StaReportSkewSummary {
 public:
  StaReportSkewSummary(const char* rpt_file_name, AnalysisMode analysis_mode,
                       unsigned n_worst);
  virtual ~StaReportSkewSummary();

  unsigned get_n_worst() const { return _n_worst; }
  auto& get_report_tbl() { return _report_tbl; }
  auto get_analysis_mode() { return _analysis_mode; }
  auto& get_report_path_skews() { return _report_path_skews; }

  virtual std::unique_ptr<StaReportTable> createReportTable(
      const char* tbl_name);
  virtual unsigned operator()(StaSeqPathData* seq_path_data);
  virtual unsigned operator()(StaSeqPathGroup* seq_path_group);
  virtual unsigned operator()(Sta* ista);

 protected:
  const char* _rpt_file_name;   //!< The report file name.
  AnalysisMode _analysis_mode;  //!< The max/min analysis mode.
  unsigned _n_worst;            //!< The top n path num.

  std::unique_ptr<StaReportTable> _report_tbl;

  std::vector<std::unique_ptr<StaReportTable>>
      _report_path_skews;  //!< The n worst path skew.
};

/**
 * @brief The skew report.
 *
 */
class StaReportSkewDetail : public StaReportSkewSummary {
 public:
  StaReportSkewDetail(const char* rpt_file_name, AnalysisMode analysis_mode,
                      unsigned n_worst);
  ~StaReportSkewDetail() override = default;

  std::unique_ptr<StaReportTable> createReportTable(
      const char* tbl_name) override;
  unsigned operator()(StaSeqPathData* seq_path_data) override;
  unsigned operator()(StaSeqPathGroup* seq_path_group) override {
    return StaReportSkewSummary::operator()(seq_path_group);
  }
  unsigned operator()(Sta* ista) override {
    return StaReportSkewSummary::operator()(ista);
  }
};

/**
 * @brief report the -from -through -to specify path.
 *
 */
class StaReportSpecifyPath {
 public:
  StaReportSpecifyPath(const char* rpt_file_name, AnalysisMode analysis_mode,
                       const char* from, const char* through, const char* to);
  ~StaReportSpecifyPath() = default;

  unsigned operator()(StaSeqPathData* seq_path_data);
  unsigned operator()(StaSeqPathGroup* seq_path_group);
  unsigned operator()(Sta* ista);

 private:
  const char* _rpt_file_name;   //!< The report file name.
  AnalysisMode _analysis_mode;  //!< The max/min analysis mode.
  const char* _from;            //!< The from pin.
  const char* _through;         //!< The through pin.
  const char* _to;              //!< The to pin.
};

}  // namespace ista
