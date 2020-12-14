#include "tests.h"

// "Global EDF scheduling of directed acyclic graphs on multiprocessor systems", Qamhieh et al. (RTNS 2013)

//TODO local offset, local deadline
/* */
bool G_EDF_Qamhieh2013_C(const Taskset& taskset, const int m){
for(const auto& task:taskset.tasks){
        if(!(task.getDeadline() <= task.getPeriod()))
            FatalError("This test requires constrained deadline tasks");
    }
}
