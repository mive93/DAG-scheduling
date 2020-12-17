#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

bool deadlineMonotonicSorting (const DAGTask& tau_x, const DAGTask& tau_y);

bool Graham1969(const Taskset& taskset, const int m); 

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

bool GP_FP_FTP_Pathan17_C(Taskset taskset, const int m); //TODO

bool GP_FP_FTP_Fonseca17_C(Taskset taskset, const int m); //TODO

//global policy, limited preemptive

bool GP_LP_FTP_Serrano16_C(Taskset taskset, const int m); //TODO

// needed TODO
// Serrano ’16 [104] -sure
// Pathan ’17 [88] - sure
// Fonseca ’17 [47] - yes
// Han ’19 [59] - yes 
// He ’19 [60] - yes 
// Fonseca ’19 [48] - yes
// Chang ’20 [36] - yes


// if you have time
// Baruah ’15 [9] - maybe no
// Parri ’15 [87] - maybe no
// Yang ’16 [124] - obi task, don't know
// Serrano ’17 [103] - maybe
// Serrano ’18 [102] - maybe






#endif /*TESTS_H*/