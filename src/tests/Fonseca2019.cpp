#include "tests.h"

//Fonseca et al. “Schedulability Analysis of DAG Tasks with Arbitrary Deadlines under Global Fixed-Priority Scheduling”.  (Real-Time Systems 2019) 


float computeCarryWorkload_C(const DAGTask& task, const float interval, const std::vector<std::pair<float, float>>& WD_UCO_y, const std::vector<std::pair<float, float>>& WD_UCI_y, const int m){
    // Algorithm 2

    float B_y = std::max(task.getLength(), task.getVolume() / m);

    float x2 = std::min(interval, B_y);
    float x1 = interval - x2;
    float CI = computeCarryInUpperBound(task, x1, WD_UCI_y );
    float CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
    float WyC = CI + CO;

    x1 = std::min(interval, B_y + (task.getPeriod() - task.R));
    x2 = interval - x1;
    CI = computeCarryInUpperBound(task, x1, WD_UCI_y );
    CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
    WyC = std::max(WyC, CI + CO );

    x1 = (task.getPeriod() - task.R);

    for(int i=WD_UCI_y.size() -1; i>=0; --i){
        x1 += WD_UCI_y[i].first;
        x2 = interval - x1;
        if(x2 >= 0){        
            CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
            CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
            WyC = std::max(WyC, CI + CO);
        }
    }

    x2= 0;
    
    for(int i=0; i<WD_UCO_y.size(); ++i){
        x2 += WD_UCO_y[i].first;
        x1 = interval - x2;
        if (x1 >= 0){
            CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
            CO = computeCarryOutUpperBound(task, x2,  WD_UCO_y);
            WyC = std::max(WyC, CI + CO);
        }
    }
    return WyC;
}

float computeCarryWorkload_A(const DAGTask& task, const float interval, const std::vector<std::pair<float, float>>& WD_UCO_y, const std::vector<std::pair<float, float>>& WD_UCI_y, const int m){
    // Algorithm 3

    float B_y = std::max(task.getLength(), task.getVolume() / m);

    float x2 = std::min(interval, B_y);
    float x1 = interval - x2;
    float CI = computeCarryInUpperBound(task, x1, WD_UCI_y );
    float CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
    float WyC = CI + CO;

    float ceil_D_over_T = std::ceil(task.getDeadline() / task.getPeriod());

    x1 = std::min(interval, B_y + ceil_D_over_T * task.getPeriod() - task.R);
    x2 = interval - x1;
    CI = computeCarryInUpperBound(task, x1, WD_UCI_y );
    CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
    WyC = std::max(WyC, CI + CO );

    for(int j = 0; j < ceil_D_over_T; ++j){
        x1 = (task.getPeriod() * j - task.R);
        for(int i=WD_UCI_y.size() -1; i>=0; --i){
            x1 += WD_UCI_y[i].first;
            x2 = interval - x1;
            if(x2 >= 0 && x1 >= 0){        
                CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
                CO = computeCarryOutUpperBound(task, x2, WD_UCO_y);
                WyC = std::max(WyC, CI + CO);
            }
        }
    }

    x2= 0;
    for(int i=0; i<WD_UCO_y.size(); ++i){
        x2 += WD_UCO_y[i].first;
        x1 = interval - x2;
        if (x1 >= 0){
            CI = computeCarryInUpperBound(task, x1, WD_UCI_y);
            CO = computeCarryOutUpperBound(task, x2,  WD_UCO_y);
            WyC = std::max(WyC, CI + CO);
        }
    }
    return WyC;
}

float computeDeltaC_B(const DAGTask& task, const float interval, const int m){
    // equation 11
    float L = task.getLength();
    float T = task.getPeriod();
    float B = std::max(L, task.getVolume() / m);
    return interval - std::max( float(0), std::floor( (interval - B) / T )) * T;
}


float interTaskWorkload_C(const DAGTask& task, const float interval,  const std::vector<std::pair<float, float>>& WD_UCO_y, const std::vector<std::pair<float, float>>& WD_UCI_y, const int m, bool constrained_deadlines = true){
    // equation 14

    float delta_c = computeDeltaC_B(task, interval, m);
    float carry_workload;
    if( constrained_deadlines) 
        carry_workload = computeCarryWorkload_C(task, delta_c, WD_UCO_y, WD_UCI_y, m);
    else
        carry_workload = computeCarryWorkload_A(task, delta_c, WD_UCO_y, WD_UCI_y, m);
    
    float T = task.getPeriod();
    float vol = task.getVolume();

    return carry_workload + std::max( float(0), std::floor( (interval - delta_c) /  T )) * vol;
}

bool GP_FP_FTP_Fonseca2019(Taskset taskset, const int m, bool constrained_deadlines){
    std::sort(taskset.tasks.begin(), taskset.tasks.end(), deadlineMonotonicSorting);

    std::vector<float> R_old (taskset.tasks.size(), 0);
    std::vector<float> R (taskset.tasks.size(), 0);

    std::vector<std::vector<std::pair<float, float>>> WD_UCO (taskset.tasks.size());
    std::vector<std::vector<std::pair<float, float>>> WD_UCI (taskset.tasks.size());
    for(int i=0; i<taskset.tasks.size(); ++i){
        R_old[i] = taskset.tasks[i].getLength();
        taskset.tasks[i].R = taskset.tasks[i].getLength();
        taskset.tasks[i].computeEFTs();

        WD_UCO[i] = computeWorkloadDistributionCO(taskset.tasks[i], i);
        WD_UCI[i] = computeWorkloadDistributionCI(taskset.tasks[i]);
    }

    for(int i=0; i<taskset.tasks.size(); ++i){

        if(R_old[i] > taskset.tasks[i].getDeadline())
            return false;

        bool init = true;
        while(!areEqual<float>(R[i], R_old[i]) && R[i] <= taskset.tasks[i].getDeadline()){
            if(!init){
                R_old[i] = R[i];
                R[i] = 0;
            }

            if(i > 0){
                for(int j=0; j<i; ++j)
                    R[i] += interTaskWorkload_C(taskset.tasks[j], R_old[i],  WD_UCO[j], WD_UCI[j], m, constrained_deadlines); 

                R[i] *= (1. / m);
                R[i] += taskset.tasks[i].getLength() + 1. / m * (taskset.tasks[i].getVolume() - taskset.tasks[i].getLength());
            }
            else
                R[i] = R_old[i];

            init = false;
        }

        if( areEqual<float>(R[i], R_old[i]))
            taskset.tasks[i].R = R[i];
        else if (R[i] > taskset.tasks[i].getDeadline())
            return false;
    }

    return true;
}
