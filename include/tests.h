#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include "DAGTask.h"
#include "Taskset.h"

bool G_EDF_Baruah2012_C(const DAGTask& task, const int m);
bool G_EDF_Baruah2012_A(const DAGTask& task, const int m);
bool G_EDF_Bonifaci2013_A(const Taskset& taskset, const int m);
bool G_DM_Bonifaci2013_A(const Taskset& taskset, const int m);
bool G_DM_Bonifaci2013_C(const Taskset& taskset, const int m);
bool G_EDF_Li2013_I(const Taskset& taskset, const int m);

#endif /*TESTS_H*/