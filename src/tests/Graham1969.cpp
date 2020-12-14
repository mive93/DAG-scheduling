#include "tests.h"

// Bounds on Multiprocessing Timing Anomalies, Garham (SIAM Journal on Applied Mathematics 1969)

bool Graham1969(const Taskset& taskset, const int m){
    for(const auto& task:taskset.tasks){
        if(task.getLength() + 1. / m * (task.getVolume() - task.getLength()) > task.getDeadline()){
            return false;
        }
    }    
    return true;
    
}
