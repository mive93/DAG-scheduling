#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>

enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

class SubTask{

    public:

    int id = 0;
    int c = 0; // WCET
    subTaskMode mode = NORMAL_T; 
    int width = 0;
    int depth = 0;
    std::vector<SubTask*> succ;
    std::vector<SubTask*> prec;
    int accWorkload = 0;

    int accWork = 0; // accumulated workload

    std::vector<int> getCondPred(){
        std::vector<int> cond_pred;
        for(const auto& p:prec){
            if(p->mode == C_SOURCE_T)
                cond_pred.push_back(p->id);
        }
        return cond_pred;
    }
   
};


#endif /* SUBTASK_H */