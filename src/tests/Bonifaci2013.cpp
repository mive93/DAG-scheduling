#include "tests.h"

// "Feasibility Analysis in the Sporadic DAG Task Model" Bonifaci et al. (ECRTS 2013)

/* Theorem 21 in the paper */
bool GP_FP_EDF_Bonifaci2013_A(const Taskset& taskset, const int m){
    float constr_contrib = 0, unconstr_contrib = 0;
    for(const auto& task:taskset.tasks){
        if(task.getPeriod() <= task.getDeadline())
            unconstr_contrib +=  task.getVolume() /  task.getPeriod();
        else
            constr_contrib +=  task.getVolume() /  task.getDeadline();

        if(task.getLength() > task.getDeadline() / 3. )
            return false;
    }

    if(unconstr_contrib + constr_contrib < (m+0.5)/3.)
        return true;
    return false;
}

/* Theorem 22 in the paper */
bool GP_FP_DM_Bonifaci2013_A(const Taskset& taskset, const int m){
    float constr_contrib = 0, unconstr_contrib = 0;
    for(const auto& task:taskset.tasks){
        if(task.getPeriod() <= 2 * task.getDeadline())
            unconstr_contrib +=  task.getVolume() /  task.getPeriod();
        else
            constr_contrib +=  task.getVolume() / (4. *  task.getDeadline());

        if(task.getLength() > task.getDeadline() / 5. )
            return false;
    }

    if(unconstr_contrib + constr_contrib < (m+0.25)/5.)
        return true;
    return false;
}

/* Theorem 23 in the paper */
bool GP_FP_DM_Bonifaci2013_C(const Taskset& taskset, const int m){
    float constr_contrib = 0, unconstr_contrib = 0;
    for(const auto& task:taskset.tasks){
        if(!(task.getDeadline() <= task.getPeriod()))
            FatalError("This test requires constrained deadline tasks");

        if(task.getPeriod() <= 2 * task.getDeadline())
            unconstr_contrib +=  task.getVolume() /  task.getPeriod();
        else
            constr_contrib +=  task.getVolume() /  task.getDeadline();

        if(task.getLength() > task.getDeadline() / 4. )
            return false;
    }

    if(unconstr_contrib + constr_contrib < (m+1./3.)/4.)
        return true;
    return false;
}