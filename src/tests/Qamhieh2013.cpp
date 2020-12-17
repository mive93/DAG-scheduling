#include "tests.h"
#include "scheduling_utils.h"

// "Global EDF scheduling of directed acyclic graphs on multiprocessor systems", Qamhieh et al. (RTNS 2013)

/* Theorem 6 in the paper */
bool GP_FP_EDF_Qamhieh2013_C(Taskset taskset, const int m){

    float cumulativeDBF = 0;
    float cumulativeCarryIn = 0;
    
    for(auto& task:taskset.tasks){
        if(!(task.getDeadline() <= task.getPeriod()))
            FatalError("This test requires constrained deadline tasks");

        task.computeLocalOffsets();
        task.computeLocalDeadlines();
    }

    for(int x=0; x<taskset.tasks.size(); ++x){
        for(int y=0; y<taskset.tasks.size(); ++y){
            for(const auto& v: taskset.tasks[y].getVertices())
                cumulativeDBF += std::max(0, demandBoundFunction(taskset.tasks[x].getDeadline(), 
                                                                 v->localD, 
                                                                 taskset.tasks[y].getPeriod(), 
                                                                 v->c ));

            if(x != y){
                for(const auto& v: taskset.tasks[y].getVertices())
                    cumulativeCarryIn += std::fmin(v->c, std::fmax(0, v->localD));
            }
        }

        if(cumulativeDBF + cumulativeCarryIn > taskset.tasks[x].getDeadline())
            return false;
    }

    return true;
}
