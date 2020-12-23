#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

bool deadlineMonotonicSorting (const DAGTask& tau_x, const DAGTask& tau_y);

bool Graham1969(const DAGTask& task, const int m); 

//global policy, fully preemptive
bool GP_FP_EDF_Baruah2012_C(const DAGTask& task, const int m);
bool GP_FP_EDF_Baruah2012_A(const DAGTask& task, const int m);

bool GP_FP_EDF_Bonifaci2013_A(const Taskset& taskset, const int m);
bool GP_FP_DM_Bonifaci2013_A(const Taskset& taskset, const int m);
bool GP_FP_DM_Bonifaci2013_C(const Taskset& taskset, const int m);

bool GP_FP_EDF_Li2013_I(const Taskset& taskset, const int m);

bool GP_FP_EDF_Qamhieh2013_C(Taskset taskset, const int m); 

bool GP_FP_EDF_Baruah2014_C(Taskset taskset, const int m); 

float workloadUpperBound(const DAGTask& task, const float t, const int m);
bool GP_FP_EDF_Melani2015_C(Taskset taskset, const int m);
bool GP_FP_FTP_Melani2015_C(Taskset taskset, const int m);

bool GP_FP_DM_Pathan2017_C(Taskset taskset, const int m); 

std::vector<std::pair<float, float>> computeWorkloadDistributionCO(const DAGTask& t, const int task_idx);
std::vector<std::pair<float, float>> computeWorkloadDistributionCI(const DAGTask& task);
float computeCarryOutUpperBound(const DAGTask& task, const int interval, const std::vector<std::pair<float, float>>& WD_UCO_y);
float computeCarryInUpperBound(const DAGTask& task, const int interval, const std::vector<std::pair<float, float>>& WD_UCI_y);
bool GP_FP_FTP_Fonseca2017_C(Taskset taskset, const int m); 

bool GP_FP_FTP_Fonseca2019(Taskset taskset, const int m, bool constrained_deadlines = true);

bool GP_FP_Han2019_C_1(DAGTask task, const std::vector<int> m); 
bool GP_FP_FTP_He2019_C(Taskset taskset, const int m); //TODO
// Qingqiang He, Nan Guan, Zhishan Guo, et al. “Intra-task priority assignmentin real-time scheduling of dag tasks on multi-cores”. In:IEEE Transactions onParallel and Distributed Systems30.10 (2019), pp. 2283–2295.
bool GP_FP_Chang2020_C(DAGTask task, const int m); //TODO
// Shuangshuang Chang et al. “Real-Time scheduling and analysis of paralleltasks  on  heterogeneous  multi-cores”.  In:Journal  of  Systems  Architecture105(2020), p. 10170

//global policy, limited preemptive

bool GP_LP_FTP_Serrano16_C(Taskset taskset, const int m); 


// if you have time (but you don't)
// Baruah ’15 [9] - maybe no
// Parri ’15 [87] - maybe no
// Yang ’16 [124] - obi task, don't know
// Serrano ’17 [103] - maybe
// Serrano ’18 [102] - maybe






#endif /*TESTS_H*/