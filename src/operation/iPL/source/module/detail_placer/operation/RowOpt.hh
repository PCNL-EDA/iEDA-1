/*
 * @Author: Shijian Chen  chenshj@pcl.ac.cn
 * @Date: 2023-03-02 14:42:58
 * @LastEditors: Shijian Chen  chenshj@pcl.ac.cn
 * @LastEditTime: 2023-03-10 10:47:18
 * @FilePath: /irefactor/src/operation/iPL/source/module/detail_refactor/operation/RowOpt.hh
 * @Description: Execute Single-Segment Clustering
 * 
 * Copyright (c) 2023 by iEDA, All Rights Reserved. 
 */
#ifndef IPL_ROWOPT_H
#define IPL_ROWOPT_H

#include <string>
#include <vector>
#include <map>

#include "config/DetailPlacerConfig.hh"
#include "database/DPDatabase.hh"
#include "DPOperator.hh"

namespace ipl {
class RowOpt
{
public:
    RowOpt() = delete;
    RowOpt(DPConfig* config, DPDatabase* database, DPOperator* dp_operator);
    RowOpt(const RowOpt&) = delete;
    RowOpt(RowOpt&&) = delete;
    ~RowOpt();

    RowOpt& operator=(const RowOpt&) = delete;
    RowOpt& operator=(RowOpt&&) = delete;

    void runRowOpt();

private:
    DPConfig* _config;
    DPDatabase* _database;
    DPOperator* _operator;

    int32_t _site_width;
    std::map<DPInterval*, DPCluster*> _interval_to_root;
    
    void updateIntervalInfo();
    void pickAndSortMovableInstList(std::vector<DPInstance*>& movable_inst_list);
    void convertInstListToClusters(std::vector<DPInstance*>& movable_inst_list);
    void updateClusterBoundList();
    void generateClusterBounds(DPCluster* cluster, std::vector<int32_t>& bound_list);
    void correctOptimalLineInInterval(std::pair<int32_t, int32_t>& optimal_line, DPInterval* interval, int32_t width);

    int32_t obtainOptimalLegalCoordiX(int32_t optimal_x, std::pair<int32_t, int32_t>& optimal_line);
    DPCluster* createCluster(DPInstance* inst, DPInterval* interval);
    void clearCluster();
    void storeCluster(DPCluster* cluster);
    void deleteCluster(DPCluster* cluster);
    void collapseClusters(DPCluster* dest_cluster, DPCluster* src_cluster);

    void resetAllInterval();

};
}
#endif