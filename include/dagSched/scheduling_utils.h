#ifndef SCHED_UTILS_H
#define SCHED_UTILS_H

#include<cmath>

int demandBoundFunction(const float interval, const float D, const float T, const float C){
    return (std::floor( (interval - D) / T ) + 1 ) * C;
}


#endif /*SCHED_UTILS_H*/