#include "dagSched/tests.h"

// Bounds on Multiprocessing Timing Anomalies, Garham (SIAM Journal on Applied Mathematics 1969)

namespace dagSched{

bool Graham1969(const DAGTask& task, const int m){
    if(task.getLength() + 1. / m * (task.getVolume() - task.getLength()) > task.getDeadline()){
        return false;
    }
    return true;
}

}