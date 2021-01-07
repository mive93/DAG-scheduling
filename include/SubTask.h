#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>
#include "utils.h"

enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

class SubTask{

    public:

    int id          = 0;    // sub-task id
    int depth       = 0;    // depth in the graph
    int width       = 0;    // width in the graph
    int gamma       = 0;    // type of core
    int core        = 0;    // id of assigned core
    int prio        = 0;    // priority of the subtask

    float c         = 0;    // WCET
    float accWork   = 0;    // accumulated workload
    float r         = 0;    // response time of the subtask
    float localO    = -1;   // local offset - Earliest Starting Time
    float localD    = -1;   // local deadline - Latest Finishing Time
    float EFT       = -1;   // Earliest Finishing Time
    float LST       = -1;   // Latest Starting Time

    subTaskMode mode = NORMAL_T;    // type of node

    std::vector<SubTask*> succ;     // successors 
    std::vector<SubTask*> pred;     // predecessors

    std::vector<int> getCondPred();

    void localOffset();
    void EasliestFinishingTime();
    void localDeadline(const float task_deadline);
    void LatestStartingTime();
};


#endif /* SUBTASK_H */