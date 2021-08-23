#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include "dagSched/DAGTask.h"
#include "dagSched/Taskset.h"

namespace dagSched{

//common
enum PartitioningNodeOrder_t {NONE, DEC_UTIL, DEC_DENS, DEC_UTIL_DEC_DENS, DEC_DENS_DEC_UTIL, REM_UTIL};
enum PartitioningCoresOrder_t {FIRST_FIT, BEST_FIT, WORST_FIT};

bool deadlineMonotonicSorting (const DAGTask& tau_x, const DAGTask& tau_y);
bool decreasingUtilizationSorting (const DAGTask& tau_x, const DAGTask& tau_y);
std::vector<int> getCandidatesProcInOrder(const std::vector<float>& proc_util, const float cur_util, const PartitioningCoresOrder_t& c_order);

bool processorsAssignment(Taskset& taskset, const int m, const PartitioningCoresOrder_t& c_order, const PartitioningNodeOrder_t& n_order);

//methods from here
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

bool GP_FP_Han2019_C_1(const DAGTask& task, const std::vector<int> m); 

bool GP_FP_He2019_C(DAGTask task, const int m);
bool GP_FP_FTP_He2019_C(Taskset taskset, const int m); 


//global policy, limited preemptive

bool GP_LP_FTP_Serrano16_C(Taskset taskset, const int m); 

//partitioned policy

class SSTask{
    public:
    std::vector<float> S;
    std::vector<float> C;
    std::vector<int> CvID;
    float Sub = 0;
    int coreId = 0;

    void print(){
        std::cout<<"Task ss (core "<<coreId<<"):"<<std::endl;
        std::cout<<"\t";
        printVector<float>(C, "C");
        std::cout<<"\t";
        printVector<float>(S, "S");
        std::cout<<"\tSub: "<<Sub<<std::endl;
    }
};

SSTask deriveSSTask(const std::vector<SubTask*>& V, const std::vector<int>& path_ss, const int core_id, const std::vector<std::vector<float>>& RTs);
bool P_FP_FTP_Fonseca2016_C(Taskset taskset, const int m, const bool joint=true);
bool P_LP_FTP_Casini2018_C(Taskset taskset, const int m);
bool P_LP_FTP_Casini2018_C_withAssignment(Taskset taskset, const int m, const PartitioningCoresOrder_t c_order);

bool G_LP_FTP_Nasri2019_C(Taskset taskset, const int m);

#ifdef BARUAH2020
bool G_LP_FTP_Baruah2020_C_exact(const DAGTask& task, const int m);
#endif

#ifdef ZAHAF2019
bool P_LP_EDF_Zahaf2019_C(const Taskset& taskset, const int m);
#endif

}

#endif /*TESTS_H*/