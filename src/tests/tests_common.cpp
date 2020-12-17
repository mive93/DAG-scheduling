#include "tests.h"

bool deadlineMonotonicSorting (const DAGTask& tau_x, const DAGTask& tau_y) { 
    return (tau_x.getDeadline() < tau_y.getDeadline()); 
}