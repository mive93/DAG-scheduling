#include "tests.h"

// "A generalized parallel task model for recurrent real-time processes" Baruah et al. (RTSS 2012)

/* Theorem 1 in the paper */
bool GP_FP_EDF_Baruah2012_C(const DAGTask& task, const int m){

    if(!(task.getDeadline() < task.getPeriod()))
        FatalError("This test requires a constrained deadline task");

    if( (m-1) * task.getLength() / task.getDeadline() + 
        2 * task.getVolume() / task.getPeriod() <= m    )
        return true;
    return false;

}

/* Theorem 3 in the paper */
bool GP_FP_EDF_Baruah2012_A(const DAGTask& task, const int m){

    if( task.getLength() <= 2./5. * task.getDeadline() && 
        task.getVolume() <= 2./5. * m * task.getPeriod()   )
        return true;
    return false;

}

//TODO implement EDFSCHEDPP