#include "tests.h"

//Maria A Serrano et al. “Response-time analysis of DAG tasks under fixed priority scheduling with limited preemptions” (DATE 2016)

float computePreemptionNumberUpperBound(const Taskset& taskset, const int x, const float interval){
    float h = 0;

    //for all high priority tasks
    for(int y=0; y<x; y++){
        h += std::ceil( interval / taskset.tasks[y].getPeriod());
    }

    return std::min( float (taskset.tasks[x].getVertices().size() - 1) , h);
}

float computeDeltaM(const Taskset& taskset, const int x, const float interval, const float m){
    std::vector<float> max_over_taus;

    //for all lower priority tasks
    for(int y=x+1; y< taskset.tasks.size(); ++y){
        std::vector<float> max_over_vs;
        auto Vy = taskset.tasks[y].getVertices();

        for(int i=0; i < Vy.size(); ++i){
            max_over_vs.push_back(Vy[i]->c);
        }

        std::sort(max_over_vs.begin(), max_over_vs.end(), std::greater<float>());
        for(int i=0; i < m && i < max_over_vs.size(); ++i)
            max_over_taus.push_back(max_over_vs[i]);
    }

    float delta = 0;
    std::sort(max_over_taus.begin(), max_over_taus.end(), std::greater<float>());
    for(int i=0; i < m && i < max_over_taus.size(); ++i)
        delta += max_over_taus[i];

    return delta;
}

float blockingWorkload(const Taskset& taskset, const int x , const float interval, const float m){

    float delta_m = computeDeltaM(taskset, x, interval, m);
    float delta_m_minus_1 = computeDeltaM(taskset, x, interval, m -1);
    float np_x = computePreemptionNumberUpperBound(taskset, x, interval);

    return delta_m + np_x * delta_m_minus_1;

}

bool GP_LP_FTP_Serrano16_C(Taskset taskset, const int m){
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    std::vector<float> R_old (taskset.tasks.size(), 0);
    std::vector<float> R (taskset.tasks.size(), 0);

    for(int i=0; i<taskset.tasks.size(); ++i){
        R_old[i] = taskset.tasks[i].getLength() + 1./m * (taskset.tasks[i].getVolume() - taskset.tasks[i].getLength());
        taskset.tasks[i].R = R_old[i];
    }

    float interf = 0;
    float blocking = 0;
    float SI = 0;

    bool at_least_one_update = true;
    bool init = true;

    while(at_least_one_update){
        at_least_one_update = false;

        for(int i=0; i<taskset.tasks.size(); ++i){

            if(R_old[i] > taskset.tasks[i].getDeadline())
                return false;

            if(!init){
                R_old[i] = R[i];
                R[i] = 0;
            }

            //for all hp
            interf = 0;
            for(int j=0; j<i; ++j)
                interf += workloadUpperBound(taskset.tasks[j], R_old[i], m);

            //for all lp
            blocking = blockingWorkload(taskset, i, R_old[i], m);

            //self interference
            SI = (taskset.tasks[i].getVolume() - taskset.tasks[i].getLength());

            //final response time
            R[i] = taskset.tasks[i].getLength() + 1. / m * SI + std::floor(1. / m * (blocking + interf));

            taskset.tasks[i].R = R[i];

            if( !areEqual<float>(R[i], R_old[i]))
                at_least_one_update = true;
            
            if (R[i] > taskset.tasks[i].getDeadline())
                return false;
        }
        init = false;
    }
    return true;
}
