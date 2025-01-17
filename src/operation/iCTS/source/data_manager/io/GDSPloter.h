#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "CtsConfig.h"
#include "CtsDBWrapper.h"
#include "CtsInstance.h"
#include "pgl.h"

namespace icts {
using std::fstream;
using std::string;

class GDSPloter {
 public:
  GDSPloter();
  explicit GDSPloter(const string &gds_file);
  GDSPloter(GDSPloter &) = default;
  ~GDSPloter() { _log_ofs.close(); }

  void plotInstances(vector<CtsInstance *> &insts);
  void insertInstance(CtsInstance *inst);

  void refPolygon(const string &name);
  void refInstance(CtsInstance *inst);

  template <typename Polygon>
  void plotPolygons(vector<Polygon> &polys, const string &name = "polyogn",
                    int layer = 0);
  template <typename Polygon>
  void insertPolygon(const Polygon &poly, const string &name = "polygon",
                     int layer = 0);

  void insertWire(const Point &begin, const Point &end, const int &layer = 0,
                  const int &width = 80);

  void head();
  void tail();
  void topBegin();
  void strBegin();
  void strEnd();

 private:
  fstream _log_ofs;
};

template <typename Polygon>
void GDSPloter::plotPolygons(vector<Polygon> &polys,
                             const string &name /*= "polyogn"*/,
                             int layer /*= 0*/) {
  head();
  size_t idx = 0;
  for (auto &poly : polys) {
    insertPolygon(poly, name + std::to_string(idx++));
  }
  tail();
}

template <typename Polygon>
void GDSPloter::insertPolygon(const Polygon &poly,
                              const string &name /* = "polygon" */,
                              int layer /* = 0*/) {
  _log_ofs << "BGNSTR" << std::endl;
  _log_ofs << "STRNAME " << name << std::endl;

  _log_ofs << "BOUNDARY" << std::endl;
  _log_ofs << "LAYER " << layer << std::endl;
  _log_ofs << "DATATYPE 0" << std::endl;
  _log_ofs << "XY" << std::endl;
  _log_ofs << poly << std::endl;
  _log_ofs << "ENDEL" << std::endl;

  _log_ofs << "ENDSTR" << std::endl;
}

}  // namespace icts