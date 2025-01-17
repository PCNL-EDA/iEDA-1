#include "DPInterval.hh"

namespace ipl{

DPInterval::DPInterval(std::string name, int32_t min_x, int32_t max_x): _name(name), _belong_row(nullptr), _min_x(min_x), _max_x(max_x), _cluster_root(nullptr)
{
    _remain_length = max_x - min_x;
}

DPInterval::~DPInterval()
{

}

bool DPInterval::checkInLine(int32_t min_x, int32_t max_x){
    return (min_x >= _min_x && max_x <= _max_x);
}

void DPInterval::updateRemainLength(int32_t incr){
    _remain_length += incr;

    // Debug
    if(_remain_length < 0){
        LOG_WARNING << "remain_length is less than zero";
    }

    if(_remain_length > (_max_x - _min_x)){
        LOG_WARNING << "remain_length is more than origin";
    }
}

void DPInterval::resetInterval(){
    _cluster_root = nullptr;
    resetRemainLength();
}

void DPInterval::resetRemainLength(){
    _remain_length = _max_x - _min_x;
}

}