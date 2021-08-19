#include "dagSched/tests.h"

// "Outstanding Paper Award: Analysis of Global EDF for Parallel Tasks" Li et al. (ECRTS 2013) 

namespace dagSched{

/* Corollary 6 in the paper*/
bool GP_FP_EDF_Li2013_I(const Taskset& taskset, const int m){
    for(const auto& task:taskset.tasks){
        if(!( areEqual<float> (task.getDeadline() , task.getPeriod())))
            FatalError("This test requires implicit deadline tasks");

        if(task.getLength() > task.getPeriod() / ( 4. - 2. / m) )
            return false;
    }

    if(taskset.getUtilization() <= m / (4. - 2. / m ))
        return true;
    return false;
}

}