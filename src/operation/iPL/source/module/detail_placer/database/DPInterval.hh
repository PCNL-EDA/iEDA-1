/*
 * @Author: Shijian Chen  chenshj@pcl.ac.cn
 * @Date: 2023-03-02 10:46:24
 * @LastEditors: Shijian Chen  chenshj@pcl.ac.cn
 * @LastEditTime: 2023-03-09 16:14:39
 * @FilePath: /irefactor/src/operation/iPL/source/module/detail_refactor/database/DPInterval.hh
 * @Description: Interval of Row
 * 
 * Copyright (c) 2023 by iEDA, All Rights Reserved. 
 */
#ifndef IPL_DPINTERVAL_H
#define IPL_DPINTERVAL_H

#include <string>
#include <vector>

#include "DPCluster.hh"

namespace ipl {
class DPRow;

class DPInterval
{
public:
    DPInterval() = delete;
    DPInterval(std::string name, int32_t min_x, int32_t max_x);
    DPInterval(const DPInterval&) = delete;
    DPInterval(DPInterval&&) = delete;
    ~DPInterval();

    DPInterval& operator=(const DPInterval&) = delete;
    DPInterval& operator=(DPInterval&&) = delete;

    // getter
    std::string get_name() const { return _name;}
    DPRow* get_belong_row() const { return _belong_row;}
    int32_t get_min_x() const { return _min_x;}
    int32_t get_max_x() const { return _max_x;}
    DPCluster* get_cluster_root() const { return _cluster_root;}
    int32_t get_remain_length() const { return _remain_length;}
    int32_t get_max_length() const { return (_max_x - _min_x);}

    // setter
    void set_belong_row(DPRow* row) { _belong_row = row;}
    void set_min_x(int32_t min_x) { _min_x = min_x;}
    void set_max_x(int32_t max_x) { _max_x = max_x;}
    void set_cluster_root(DPCluster* cluster){ _cluster_root = cluster;}

    // function
    bool checkInLine(int32_t min_x, int32_t max_x);
    void updateRemainLength(int32_t incr);

    void resetInterval();

private:
    std::string _name;  /* row_index + segment_index */
    DPRow* _belong_row;

    int32_t _min_x;
    int32_t _max_x;

    DPCluster* _cluster_root;
    int32_t _remain_length;

    void resetRemainLength();

};
}
#endif