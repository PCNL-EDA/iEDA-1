#pragma once
/**
 * @project		iDB
 * @file		layout_write.h
 * @author		Yell
 * @date		25/05/2021
 * @version		0.1
 * @description


        There is a data builder to write data file from data structure.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <string>
#include <vector>

#include "../../def_builder/def_service/def_service.h"
#include "header.h"

namespace idb {

using std::string;
using std::vector;

class LayoutWrite
{
 public:
  LayoutWrite(IdbLayout* layout);
  ~LayoutWrite();

  // getter
  IdbLayout* get_layout() { return _layout; };

  // writer
  int32_t save_manufacture_grid();
  int32_t save_units();
  int32_t save_die();
  int32_t save_core();
  int32_t save_layers();
  int32_t save_sites();
  int32_t save_rows();
  int32_t save_gcell_grid_list();
  int32_t save_track_grid_list();
  int32_t save_cell_master_list();
  int32_t save_via_list();
  int32_t save_via_rule_list();

  void set_start_time(clock_t time) { _start_time = time; }
  void set_end_time(clock_t time) { _end_time = time; }
  float time_eclips() { return (float(_end_time - _start_time)) / CLOCKS_PER_MS; }

  // operator
  bool writeLayout(const char* folder);

 private:
  IdbLayout* _layout = nullptr;
  const char* _folder;
  int32_t _index = 0;
  clock_t _start_time = 0;
  clock_t _end_time = 0;
};
}  // namespace idb
