#ifndef SUBTASK_H
#define SUBTASK_H

#include <vector>
#include "utils.h"

enum subTaskMode {NORMAL_T, C_INTERN_T, C_SOURCE_T, C_SINK_T};

class SubTask{

    public:

    int id          = 0; // sub-task id
    float c         = 0; // WCET
    int width       = 0; // width in the graph
    int depth       = 0; // depth in the graph
    float accWork   = 0; // accumulated workload
    float r         = 0; // response time of the subtask

    float localO = -1; // local offset - EST
    float localD = -1; // local deadline - LFT

    float EFT = -1;
    float LST = -1;

    subTaskMode mode = NORMAL_T;    // type of node

    std::vector<SubTask*> succ;     // successors 
    std::vector<SubTask*> pred;    // predecessors

    std::vector<int> getCondPred(){
        std::vector<int> cond_pred;
        for(const auto& p:pred){
            if(p->mode == C_SOURCE_T)
                cond_pred.push_back(p->id);
        }
        return cond_pred;
    }

    void localOffset(){
        if(pred.size() == 0)
            localO = 0;
        else{
            localO = 0;
            float temp_local_o = 0;
            for(int i=0; i<pred.size();++i){
                temp_local_o = pred[i]->localO + pred[i]->c;
                if(temp_local_o > localO) localO = temp_local_o;
            }
        }
    }

    void EasliestFinishingTime(){
        if(localO == -1)
            FatalError("Requires local offsets to be computed");
        EFT = localO + c;
    }

    void localDeadline(const float task_deadline){
        if(succ.size() == 0)
            localD = task_deadline;
        else{
            localD = 99999;
            float temp_local_d = 0;
            for(int i=0; i<succ.size();++i){
                temp_local_d = succ[i]->localD - succ[i]->c;
                if(temp_local_d < localD) localD = temp_local_d;
            }
        }
    }

    void LatestStartingTime(){
        if(localD == -1)
            FatalError("Requires local deadlines to be computed");
        LST = localD - c;
    }
   
};


#endif /* SUBTASK_H */