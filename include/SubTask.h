#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>

enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

class SubTask{

    public:

    int id      = 0; // sub-task id
    int c       = 0; // WCET
    int width   = 0; // width in the graph
    int depth   = 0; // depth in the graph
    int accWork = 0; // accumulated workload

    int localO = 0; // local offset
    int localD = 0; // local deadline

    subTaskMode mode = NORMAL_T;    // type of node

    std::vector<SubTask*> desc;     // descendants 
    std::vector<SubTask*> ancst;    // ancestors

    std::vector<int> getCondPred(){
        std::vector<int> cond_pred;
        for(const auto& p:ancst){
            if(p->mode == C_SOURCE_T)
                cond_pred.push_back(p->id);
        }
        return cond_pred;
    }
   
};


#endif /* SUBTASK_H */